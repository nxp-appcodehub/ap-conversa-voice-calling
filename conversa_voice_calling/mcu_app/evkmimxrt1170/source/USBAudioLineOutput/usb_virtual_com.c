/*
 * Copyright (c) 2015 - 2016, Freescale Semiconductor, Inc.
 * Copyright 2016,2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "appGlobal.h"

#include "fsl_device_registers.h"
#include "clock_config.h"
#include "board.h"
#include "PL_platformTypes.h"
#include <stdio.h>
#include <stdlib.h>

#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"

#include "usb_device_class.h"
#include "usb_device_audio.h"
#include "usb_device_cdc_acm.h"
#include "usb_device_ch9.h"

#include "fsl_debug_console.h"

#include "usb_device_descriptor.h"

#include "usb_audio_unified.h"

#include <stdarg.h>

#include "usb_virtual_com.h"

// memory section replacement
#include <cr_section_macros.h>


/*******************************************************************************
* Definitions
******************************************************************************/
//if the following is not defined, the checking is simply checksum
#define TuningComAudioIsCrc32
//disable the next line to have data packet interpreted like read/write MCU memory from the host
//#define ComDataLoopBack
//#define LetComFeedbackAudioData
#define LetComConnectTuningTool
//#define LetComFeedbackMcpsValues
/*******************************************************************************
* Variables
******************************************************************************/
extern usb_device_endpoint_struct_t g_cdcVcomDicEndpoints[];


uint32_t InvertU32(uint32_t v)
{
	uint8_t a,b,c,d;
	a=(v>> 0)&0xff;
	b=(v>> 8)&0xff;
	c=(v>>16)&0xff;
	d=(v>>24)&0xff;
	return(a*0x1000000 + b*0x10000 + c*0x100 + d);
}
void crc32(uint32_t *crc, PL_UINT32 dta)
{int i;
	dta=InvertU32(dta);
	for (i=0; i<32; i++)
	{
		*crc=(*crc^dta)&0x80000000?(*crc<<1)^0x04C11DB7:(*crc<<1);
		dta<<=1;
	}
}


/* Line coding of cdc device */
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static uint8_t s_lineCoding[LINE_CODING_SIZE] = {
    /* E.g. 0x00,0xC2,0x01,0x00 : 0x0001C200 is 115200 bits per second */
    (LINE_CODING_DTERATE >> 0U) & 0x000000FFU,
    (LINE_CODING_DTERATE >> 8U) & 0x000000FFU,
    (LINE_CODING_DTERATE >> 16U) & 0x000000FFU,
    (LINE_CODING_DTERATE >> 24U) & 0x000000FFU,
    LINE_CODING_CHARFORMAT,
    LINE_CODING_PARITYTYPE,
    LINE_CODING_DATABITS};

/* Abstract state of cdc device */
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static uint8_t s_abstractState[COMM_FEATURE_DATA_SIZE] = {(STATUS_ABSTRACT_STATE >> 0U) & 0x00FFU,
                                                          (STATUS_ABSTRACT_STATE >> 8U) & 0x00FFU};

/* Country code of cdc device */
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static uint8_t s_countryCode[COMM_FEATURE_DATA_SIZE] = {(COUNTRY_SETTING >> 0U) & 0x00FFU,
                                                        (COUNTRY_SETTING >> 8U) & 0x00FFU};

/* CDC ACM information */
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static usb_cdc_acm_info_t s_usbCdcAcmInfo;
/* Data buffer for receiving and sending*/
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static uint8_t s_currRecvBuf[DATA_BUFF_SIZE + 64];

#ifdef LetComFeedbackAudioData
	USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) uint8_t VComSendBuf[DATA_BUFF_SIZE * 4 + 64];		//each time VCOM send upstream audio data --- 64 samples *6 ch * 4 bytes = 3 sectors
#else
	USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) uint8_t VComSendBuf[DATA_BUFF_SIZE + 64];
#endif


volatile static uint32_t s_recvSize = 0;
volatile uint32_t VCom_sendSize = 0;
volatile static usb_device_composite_struct_t *g_deviceComposite;
void USB_DeviceCdcVcomTask_ConnectTuningTool(void);

/*******************************************************************************
* Code
******************************************************************************/
/*!
 * @brief CDC class specific callback function.
 *
 * This function handles the CDC class specific requests.
 *
 * @param handle          The CDC ACM class handle.
 * @param event           The CDC ACM class event type.
 * @param param           The parameter of the class specific request.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
__RAMFUNC(RAM) // force to put the code instruction of this function in RAM (FLEXRAMRAM_ITC)
usb_status_t USB_DeviceCdcVcomCallback(class_handle_t handle, uint32_t event, void *param)
{
    usb_status_t error = kStatus_USB_Error;
    uint32_t len;
    uint8_t *uartBitmap;
    usb_cdc_acm_info_t *acmInfo = &s_usbCdcAcmInfo;
    usb_device_cdc_acm_request_param_struct_t *acmReqParam;
    usb_device_endpoint_callback_message_struct_t *epCbParam;
    acmReqParam = (usb_device_cdc_acm_request_param_struct_t *)param;
    epCbParam = (usb_device_endpoint_callback_message_struct_t *)param;
    switch (event)
    {
        case kUSB_DeviceCdcEventSendResponse:
        {
            if ((epCbParam->length != 0) && (!(epCbParam->length % g_cdcVcomDicEndpoints[0].maxPacketSize)))
            {
                /* If the last packet is the size of endpoint, then send also zero-ended packet,
                 ** meaning that we want to inform the host that we do not have any additional
                 ** data, so it can flush the output.
                 */
                error = USB_DeviceCdcAcmSend(handle, USB_CDC_VCOM_DIC_BULK_IN_ENDPOINT, NULL, 0);
            }
            else if ((1 == g_deviceComposite->cdcVcom.attach) && (1 == g_deviceComposite->cdcVcom.startTransactions))
            {
                if ((epCbParam->buffer != NULL) || ((epCbParam->buffer == NULL) && (epCbParam->length == 0)))
                {
                    /* User: add your own code for send complete event */
                    /* Schedule buffer for next receive event */
                    error = USB_DeviceCdcAcmRecv(handle, USB_CDC_VCOM_DIC_BULK_OUT_ENDPOINT, s_currRecvBuf,
                                                 g_cdcVcomDicEndpoints[0].maxPacketSize);
                }
            }
            else
            {
            }
        }
        break;
        case kUSB_DeviceCdcEventRecvResponse:
        {
            if ((1 == g_deviceComposite->cdcVcom.attach) && (1 == g_deviceComposite->cdcVcom.startTransactions))
            {
                s_recvSize = epCbParam->length;

                if (!s_recvSize)
                {
                    /* Schedule buffer for next receive event */
                    error = USB_DeviceCdcAcmRecv(handle, USB_CDC_VCOM_DIC_BULK_OUT_ENDPOINT, s_currRecvBuf,
                                                 g_cdcVcomDicEndpoints[0].maxPacketSize);
                }

				#ifdef LetComConnectTuningTool
				USB_DeviceCdcVcomTask_ConnectTuningTool();
				#endif
            }
        }
        break;
        case kUSB_DeviceCdcEventSerialStateNotif:
            ((usb_device_cdc_acm_struct_t *)handle)->hasSentState = 0;
            error = kStatus_USB_Success;
            break;
        case kUSB_DeviceCdcEventSendEncapsulatedCommand:
            break;
        case kUSB_DeviceCdcEventGetEncapsulatedResponse:
            break;
        case kUSB_DeviceCdcEventSetCommFeature:
            if (USB_DEVICE_CDC_FEATURE_ABSTRACT_STATE == acmReqParam->setupValue)
            {
                if (1 == acmReqParam->isSetup)
                {
                    *(acmReqParam->buffer) = s_abstractState;
                }
                else
                {
                    *(acmReqParam->length) = 0;
                }
            }
            else if (USB_DEVICE_CDC_FEATURE_COUNTRY_SETTING == acmReqParam->setupValue)
            {
                if (1 == acmReqParam->isSetup)
                {
                    *(acmReqParam->buffer) = s_countryCode;
                }
                else
                {
                    *(acmReqParam->length) = 0;
                }
            }
            else
            {
            }
            error = kStatus_USB_Success;
            break;
        case kUSB_DeviceCdcEventGetCommFeature:
            if (USB_DEVICE_CDC_FEATURE_ABSTRACT_STATE == acmReqParam->setupValue)
            {
                *(acmReqParam->buffer) = s_abstractState;
                *(acmReqParam->length) = COMM_FEATURE_DATA_SIZE;
            }
            else if (USB_DEVICE_CDC_FEATURE_COUNTRY_SETTING == acmReqParam->setupValue)
            {
                *(acmReqParam->buffer) = s_countryCode;
                *(acmReqParam->length) = COMM_FEATURE_DATA_SIZE;
            }
            else
            {
            }
            error = kStatus_USB_Success;
            break;
        case kUSB_DeviceCdcEventClearCommFeature:
            break;
        case kUSB_DeviceCdcEventGetLineCoding:
            *(acmReqParam->buffer) = s_lineCoding;
            *(acmReqParam->length) = LINE_CODING_SIZE;
            error = kStatus_USB_Success;
            break;
        case kUSB_DeviceCdcEventSetLineCoding:
        {
            if (1 == acmReqParam->isSetup)
            {
                *(acmReqParam->buffer) = s_lineCoding;
            }
            else
            {
                *(acmReqParam->length) = 0;
            }
        }
            error = kStatus_USB_Success;
            break;
        case kUSB_DeviceCdcEventSetControlLineState:
        {
            s_usbCdcAcmInfo.dteStatus = acmReqParam->setupValue;
            /* activate/deactivate Tx carrier */
            if (acmInfo->dteStatus & USB_DEVICE_CDC_CONTROL_SIG_BITMAP_CARRIER_ACTIVATION)
            {
                acmInfo->uartState |= USB_DEVICE_CDC_UART_STATE_TX_CARRIER;
            }
            else
            {
                acmInfo->uartState &= (uint16_t)~USB_DEVICE_CDC_UART_STATE_TX_CARRIER;
            }

            /* activate carrier and DTE. Com port of terminal tool running on PC is open now */
            if (acmInfo->dteStatus & USB_DEVICE_CDC_CONTROL_SIG_BITMAP_DTE_PRESENCE)
            {
                acmInfo->uartState |= USB_DEVICE_CDC_UART_STATE_RX_CARRIER;
            }
            /* Com port of terminal tool running on PC is closed now */
            else
            {
                acmInfo->uartState &= (uint16_t)~USB_DEVICE_CDC_UART_STATE_RX_CARRIER;
            }

            /* Indicates to DCE if DTE is present or not */
            acmInfo->dtePresent = (acmInfo->dteStatus & USB_DEVICE_CDC_CONTROL_SIG_BITMAP_DTE_PRESENCE) ? true : false;

            /* Initialize the serial state buffer */
            acmInfo->serialStateBuf[0] = NOTIF_REQUEST_TYPE;                /* bmRequestType */
            acmInfo->serialStateBuf[1] = USB_DEVICE_CDC_NOTIF_SERIAL_STATE; /* bNotification */
            acmInfo->serialStateBuf[2] = 0x00;                              /* wValue */
            acmInfo->serialStateBuf[3] = 0x00;
            acmInfo->serialStateBuf[4] = 0x00; /* wIndex */
            acmInfo->serialStateBuf[5] = 0x00;
            acmInfo->serialStateBuf[6] = UART_BITMAP_SIZE; /* wLength */
            acmInfo->serialStateBuf[7] = 0x00;
            /* Notify to host the line state */
            acmInfo->serialStateBuf[4] = acmReqParam->interfaceIndex;
            /* Lower byte of UART BITMAP */
            uartBitmap = (uint8_t *)&acmInfo->serialStateBuf[NOTIF_PACKET_SIZE + UART_BITMAP_SIZE - 2];
            uartBitmap[0] = acmInfo->uartState & 0xFFu;
            uartBitmap[1] = (acmInfo->uartState >> 8) & 0xFFu;
            len = (uint32_t)(NOTIF_PACKET_SIZE + UART_BITMAP_SIZE);
            if (0 == ((usb_device_cdc_acm_struct_t *)handle)->hasSentState)
            {
                error =
                    USB_DeviceCdcAcmSend(handle, USB_CDC_VCOM_CIC_INTERRUPT_IN_ENDPOINT, acmInfo->serialStateBuf, len);
                if (kStatus_USB_Success != error)
                {
                    usb_echo("kUSB_DeviceCdcEventSetControlLineState error!");
                }
                ((usb_device_cdc_acm_struct_t *)handle)->hasSentState = 1;
            }

            /* Update status */
            if (acmInfo->dteStatus & USB_DEVICE_CDC_CONTROL_SIG_BITMAP_CARRIER_ACTIVATION)
            {
                /*  To do: CARRIER_ACTIVATED */
            }
            else
            {
                /* To do: CARRIER_DEACTIVATED */
            }
            if (acmInfo->dteStatus & USB_DEVICE_CDC_CONTROL_SIG_BITMAP_DTE_PRESENCE)
            {
                /* DTE_ACTIVATED */
                if (1 == g_deviceComposite->cdcVcom.attach)
                {
                    g_deviceComposite->cdcVcom.startTransactions = 1;
                }
            }
            else
            {
                /* DTE_DEACTIVATED */
                if (1 == g_deviceComposite->cdcVcom.attach)
                {
                    g_deviceComposite->cdcVcom.startTransactions = 0;
                }
            }
        }
        break;
        case kUSB_DeviceCdcEventSendBreak:
            break;
        default:
            break;
    }

    return error;
}




/*!
 * @brief Application task function.
 *
 * This function runs the task for application.
 *
 * @return None.
 */
#ifdef ComDataLoopBack
void USB_DeviceCdcVcomTask(void)
{
    usb_status_t error = kStatus_USB_Error;
    if ((1 == g_deviceComposite->cdcVcom.attach) && (1 == g_deviceComposite->cdcVcom.startTransactions))
    {
        /* User Code */
        if ((0 != s_recvSize) && (0xFFFFFFFFU != s_recvSize))
        {
            int32_t i;

            /* Copy Buffer to Send Buff */
            for (i = 0; i < s_recvSize; i++)
            {
                VComSendBuf[VCom_sendSize++] = s_currRecvBuf[i];
            }
            s_recvSize = 0;
        }

        if (VCom_sendSize)
        {
            uint32_t size = VCom_sendSize;
            VCom_sendSize = 0;

            if(s_currRecvBuf[0]<0x34)
            {
                error = USB_DeviceCdcAcmSend(g_deviceComposite->cdcVcom.cdcAcmHandle, USB_CDC_VCOM_DIC_BULK_IN_ENDPOINT, (U8 *)0x20000000, 0x180);
            }else
            {
                error = USB_DeviceCdcAcmSend(g_deviceComposite->cdcVcom.cdcAcmHandle, USB_CDC_VCOM_DIC_BULK_IN_ENDPOINT, (U8 *)VComSendBuf, size);
            }


            if (error != kStatus_USB_Success)
            {
                /* Failure to send Data Handling code here */
            	error=0;
            }
        }
    }
}
#endif






#ifdef LetComFeedbackVolumeControlInfo
extern usb_device_composite_struct_t *g_deviceAudioComposite;
void USB_DeviceCdcVcomTask(void)
{
	S16 CurVol;
	S16 MaxVol;
	S16 MinVol;
	S16 ResVol;
	S16 CurMute;
	S16 NumSubRange;

	CurMute=g_deviceAudioComposite->audioUnified.curMute20;
	CurVol=(g_deviceAudioComposite->audioUnified.curVolume20[1]<<8)|(g_deviceAudioComposite->audioUnified.curVolume20[0]);
	MinVol=g_deviceAudioComposite->audioUnified.volumeControlRange.wMIN;
	MaxVol=g_deviceAudioComposite->audioUnified.volumeControlRange.wMAX;
	ResVol=g_deviceAudioComposite->audioUnified.volumeControlRange.wRES;
	NumSubRange=g_deviceAudioComposite->audioUnified.volumeControlRange.wNumSubRanges;

    usb_status_t error = kStatus_USB_Error;
    if ((1 == g_deviceComposite->cdcVcom.attach) && (1 == g_deviceComposite->cdcVcom.startTransactions))
    {
        /* User Code */
        if ((0 != s_recvSize) && (0xFFFFFFFFU != s_recvSize))
        {
            int32_t i;

            /* Copy Buffer to Send Buff */
            for (i = 0; i < s_recvSize; i++)
            {
                VComSendBuf[VCom_sendSize++] = s_currRecvBuf[i];
            }
            s_recvSize = 0;
        }

        if (VCom_sendSize)
        {
            uint32_t size = VCom_sendSize;
            VCom_sendSize = 0;

            sprintf(VComSendBuf,"%d,%d,%d,%d,%d,%d\n",CurMute,CurVol,MaxVol,MinVol,ResVol,NumSubRange);
            error = USB_DeviceCdcAcmSend(g_deviceComposite->cdcVcom.cdcAcmHandle, USB_CDC_VCOM_DIC_BULK_IN_ENDPOINT, VComSendBuf, strlen(VComSendBuf));

            if (error != kStatus_USB_Success)
            {
                /* Failure to send Data Handling code here */
            	error=0;
            }
        }
    }

}

#endif



#ifdef LetComFeedbackUsbUpDnStreamBufLevel
U32 VComReportValue_UpStrmBufLevelInSamples=0;
U32 VComReportValue_MicUpStreamPktLengthInSamples=0;
U32 VComReportValue_DnStrmBufLevelInSamples=0;
U32 VComReportValue_DnStrmPktLengthInSamples=0;
U32 VComReportValue_FeedbackEpValue=(AUDIO_OUT_SAMPLING_RATE_KHZ*1024);
void USB_DeviceCdcVcomTask(void)
{
    usb_status_t error = kStatus_USB_Error;
    if ((1 == g_deviceComposite->cdcVcom.attach) && (1 == g_deviceComposite->cdcVcom.startTransactions))
    {
        /* User Code */
        if ((0 != s_recvSize) && (0xFFFFFFFFU != s_recvSize))
        {
            int32_t i;

            /* Copy Buffer to Send Buff */
            for (i = 0; i < s_recvSize; i++)
            {
                VComSendBuf[VCom_sendSize++] = s_currRecvBuf[i];
            }
            s_recvSize = 0;
        }

        if (VCom_sendSize)
        {
            uint32_t size = VCom_sendSize;
            VCom_sendSize = 0;

            sprintf(VComSendBuf,"%d,%d,%d,%d,%d\n",VComReportValue_UpStrmBufLevelInSamples,VComReportValue_MicUpStreamPktLengthInSamples,VComReportValue_DnStrmBufLevelInSamples,VComReportValue_DnStrmPktLengthInSamples,VComReportValue_FeedbackEpValue);

            error = USB_DeviceCdcAcmSend(g_deviceComposite->cdcVcom.cdcAcmHandle, USB_CDC_VCOM_DIC_BULK_IN_ENDPOINT, VComSendBuf, strlen(VComSendBuf));


            if (error != kStatus_USB_Success)
            {
                /* Failure to send Data Handling code here */
            	error=0;
            }
        }
    }
}
#endif

#ifdef LetComFeedbackMcpsValues
void USB_DeviceCdcVcomTask(void)
{
    usb_status_t error = kStatus_USB_Error;
    if ((1 == g_deviceComposite->cdcVcom.attach) && (1 == g_deviceComposite->cdcVcom.startTransactions))
    {
        /* User Code */
        if ((0 != s_recvSize) && (0xFFFFFFFFU != s_recvSize))
        {
            int32_t i;

            /* Copy Buffer to Send Buff */
            for (i = 0; i < s_recvSize; i++)
            {
                VComSendBuf[VCom_sendSize++] = s_currRecvBuf[i];
            }
            s_recvSize = 0;
        }

        if (VCom_sendSize)
        {
            uint32_t size = VCom_sendSize;
            VCom_sendSize = 0;

            error = USB_DeviceCdcAcmSend(g_deviceComposite->cdcVcom.cdcAcmHandle, USB_CDC_VCOM_DIC_BULK_IN_ENDPOINT, VComSendBuf, size);


            if (error != kStatus_USB_Success)
            {
                /* Failure to send Data Handling code here */
            	error=0;
            }
        }
    }
}


#endif

#ifdef LetComFeedbackAudioData
void USB_DeviceCdcVcomTask(void)
{
    usb_status_t error = kStatus_USB_Error;
    if ((1 == g_deviceComposite->cdcVcom.attach) && (1 == g_deviceComposite->cdcVcom.startTransactions))
    {
        /* User Code */
        if ((0 != s_recvSize) && (0xFFFFFFFFU != s_recvSize))
        {
            int32_t i;

            /* Copy Buffer to Send Buff */
            for (i = 0; i < s_recvSize; i++)
            {
                VComSendBuf[VCom_sendSize++] = s_currRecvBuf[i];
            }
            s_recvSize = 0;
        }

        if (VCom_sendSize)
        {
            uint32_t size = VCom_sendSize;
            VCom_sendSize = 0;

            error = USB_DeviceCdcAcmSend(g_deviceComposite->cdcVcom.cdcAcmHandle, USB_CDC_VCOM_DIC_BULK_IN_ENDPOINT, VComSendBuf, size);


            if (error != kStatus_USB_Success)
            {
                /* Failure to send Data Handling code here */
            	error=0;
            }
        }
    }
}


#endif
#ifdef LetComConnectTuningTool

#define MaxVComTransferPayloadSizeInByte	HS_CDC_VCOM_BULK_OUT_PACKET_SIZE
#define ComPacketHead_RdMcuMemory		0x11110000
#define ComPacketHead_WrMcuMemory		0x11110001
#define ComPacketHead_McuRdReturn		0x21110000
#define ComPacketHead_McuWrReturn		0x21110001


uint8_t NeedToClearWhatIsReceivedFromComHostJustNow;

__RAMFUNC(RAM) // force to put the code instruction of this function in RAM (FLEXRAMRAM_ITC)
void USB_DeviceCdcVcomTask_ConnectTuningTool(void)
{
	uint32_t *p1;
	uint32_t *p2;
	uint32_t AuditU32;
	uint32_t addr;
	uint32_t WantedDataLenInBytes;
	int i;

    usb_status_t error = kStatus_USB_Error;
    if ((1 == g_deviceComposite->cdcVcom.attach) && (1 == g_deviceComposite->cdcVcom.startTransactions))
    {
        /* User Code */
        if ((0 != s_recvSize) && (0xFFFFFFFFU != s_recvSize))
        {
            int32_t i;

			#ifdef TuningComAudioIsCrc32
				uint32_t Crc32Result;
			#endif


			NeedToClearWhatIsReceivedFromComHostJustNow=1;
            p1=(uint32_t *)s_currRecvBuf;
            if(*p1==ComPacketHead_RdMcuMemory)
            {	//read mcu memory
            	p1++;
            	addr=*p1++;
            	WantedDataLenInBytes=*p1++;
            	AuditU32=*p1++;

			#ifdef TuningComAudioIsCrc32
	            p2=(uint32_t *)s_currRecvBuf;
				Crc32Result=0xffffffff;
				for(i=0;i<3;i++)
				    crc32(&Crc32Result,*p2++);
            	if(AuditU32==Crc32Result)
			#else
               	if(!(ComPacketHead_RdMcuMemory+addr+WantedDataLenInBytes+AuditU32))
			#endif
            	{	//check AuditU32 audit succ
            		if(WantedDataLenInBytes>MaxVComTransferPayloadSizeInByte) WantedDataLenInBytes=MaxVComTransferPayloadSizeInByte;
                    p2=(uint32_t *)VComSendBuf;
                    *p2++=ComPacketHead_McuRdReturn;
                    *p2++=addr;
                    *p2++=WantedDataLenInBytes;
                    memcpy(p2,(uint8_t *)addr,WantedDataLenInBytes);
                    VCom_sendSize=16+WantedDataLenInBytes;
            	}else
        			PRINTF("RT1060: COM received data packet audit fail --- RdMcuMemory \r\n");
            }
            if(*p1==ComPacketHead_WrMcuMemory)
            {	//write mcu memory
            	p1++;
            	addr=*p1++;
            	WantedDataLenInBytes=*p1++;
			#ifdef TuningComAudioIsCrc32
				p2=(uint32_t *)s_currRecvBuf;
				AuditU32=*(p2+WantedDataLenInBytes/4+3);
				Crc32Result=0xffffffff;
				for(i=0;i<(WantedDataLenInBytes/4+3);i++)
					crc32(&Crc32Result,*p2++);
				if(AuditU32==Crc32Result)
			#else
				p1=(uint32_t *)s_currRecvBuf;
				AuditU32=0;
				for(i=0;i<((WantedDataLenInBytes+16)/4);i++)
					AuditU32+=*p1++;
				p1=(uint32_t *)s_currRecvBuf;
				p1+=3;
				if(!AuditU32)
			#endif
                {	//check AuditU32 audit succ
            		if(WantedDataLenInBytes>MaxVComTransferPayloadSizeInByte) WantedDataLenInBytes=MaxVComTransferPayloadSizeInByte;
					p1=(uint32_t *)s_currRecvBuf;
					p1+=3;
                	memcpy((uint8_t *)addr,(uint8_t *)p1,WantedDataLenInBytes);
                    p2=(uint32_t *)VComSendBuf;
                    *p2++=ComPacketHead_McuWrReturn;
                    *p2++=addr;
                    *p2++=WantedDataLenInBytes;
                    VCom_sendSize=16;
                }else
        			PRINTF("RT1060: COM received data packet audit fail --- WrMcuMemory \r\n");
            }
            s_recvSize = 0;

        }else
        {
        	NeedToClearWhatIsReceivedFromComHostJustNow=0;
        }

        if (VCom_sendSize)
        {


            uint32_t size = VCom_sendSize;
            VCom_sendSize = 0;
			#ifdef TuningComAudioIsCrc32
				uint32_t Crc32Result;
	            p2=(uint32_t *)VComSendBuf;
				Crc32Result=0xffffffff;
				for(i=0;i<(size/4-1);i++)
				    crc32(&Crc32Result,*p2++);
	            *p2=Crc32Result;
			#else
				AuditU32=0;
				p2=(uint32_t *)VComSendBuf;
				for(i=0;i<(size-4)/4;i++)
					AuditU32+=*p2++;
				*p2=0-AuditU32;
			#endif
            error = USB_DeviceCdcAcmSend(g_deviceComposite->cdcVcom.cdcAcmHandle, USB_CDC_VCOM_DIC_BULK_IN_ENDPOINT, VComSendBuf, size);

            if (error != kStatus_USB_Success)
            {
                /* Failure to send Data Handling code here */
    			PRINTF("RT1060: COM send bytes error --- RdMcuMemory \r\n");
            }
			NeedToClearWhatIsReceivedFromComHostJustNow=0;

        }else
        {
        	if(NeedToClearWhatIsReceivedFromComHostJustNow)
        	{
				error = USB_DeviceCdcAcmSend(g_deviceComposite->cdcVcom.cdcAcmHandle, USB_CDC_VCOM_DIC_BULK_IN_ENDPOINT, VComSendBuf, 0);
				NeedToClearWhatIsReceivedFromComHostJustNow=0;
        	}
        }
    }
}
#endif
/*!
 * @brief Virtual COM device set configuration function.
 *
 * This function sets configuration for CDC class.
 *
 * @param handle The CDC ACM class handle.
 * @param configure The CDC ACM class configure index.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceCdcVcomSetConfigure(class_handle_t handle, uint8_t configure)
{
    if (USB_COMPOSITE_CONFIGURE_INDEX == configure)
    {
        g_deviceComposite->cdcVcom.attach = 1;
        /* Schedule buffer for receive */
        USB_DeviceCdcAcmRecv(g_deviceComposite->cdcVcom.cdcAcmHandle, USB_CDC_VCOM_DIC_BULK_OUT_ENDPOINT, s_currRecvBuf,
                             g_cdcVcomDicEndpoints[0].maxPacketSize);
    }
    return kStatus_USB_Success;
}

/*!
 * @brief Virtual COM device initialization function.
 *
 * This function initializes the device with the composite device class information.
 *
 * @param deviceComposite The pointer to the composite device structure.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceCdcVcomInit(usb_device_composite_struct_t *deviceComposite)
{
    g_deviceComposite = deviceComposite;
    return kStatus_USB_Success;
}



#if 0
#if SDK_DEBUGCONSOLE
extern void DbgConsole_PrintCallback(char *buf, int32_t *indicator, char dbgVal, int len);
#endif

int UsbCom_Printf(const char *formatString, ...)
{
    va_list ap;
    int logLength = 0, dbgResult = 0;

    if ((1 == g_deviceComposite->cdcVcom.attach) && (1 == g_deviceComposite->cdcVcom.startTransactions))
    {
        va_start(ap, formatString);
        /* format print log first */
        logLength = StrFormatPrintf(formatString, ap, VComSendBuf, DbgConsole_PrintCallback);
        /* print log */
        //dbgResult = DbgConsole_SendDataReliable((uint8_t *)printBuf, (size_t)logLength);

        if (logLength)
        {
            uint32_t size = logLength;
            logLength = 0;

            USB_DeviceCdcAcmSend(g_deviceComposite->cdcVcom.cdcAcmHandle, USB_CDC_VCOM_DIC_BULK_IN_ENDPOINT, VComSendBuf, size);
        }

        va_end(ap);
    }


    return dbgResult;
}
#endif



