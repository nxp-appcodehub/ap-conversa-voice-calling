/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


//Standard headers
#include <stdint.h>

//Platform headers
#include "board.h"
#include "clock_config.h"

//fsl driver headers
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_crc.h"

//Application headers
#include "appGlobal.h"
#include "usb_init.h"



/*******************************************************************************
 * Definitions
 ******************************************************************************/

void USB_DeviceCdcVcomTask_ConnectTuningTool(void);


/*******************************************************************************
 * Variables
 ******************************************************************************/
extern usb_device_endpoint_struct_t g_cdcVcomDicEndpoints[];

__attribute__((__section__(".data.$SRAM_ControlBlock")))
PL_UINT32 ControlBlockAddr;



/* Line coding of cdc device */
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
static uint8_t s_lineCoding[LINE_CODING_SIZE] = {
    /* E.g. 0x00,0xC2,0x01,0x00 : 0x0001C200 is 115200 bits per second */
    (LINE_CODING_DTERATE >> 0U) & 0x000000FFU,
    (LINE_CODING_DTERATE >> 8U) & 0x000000FFU,
    (LINE_CODING_DTERATE >> 16U) & 0x000000FFU,
    (LINE_CODING_DTERATE >> 24U) & 0x000000FFU,
    LINE_CODING_CHARFORMAT,
    LINE_CODING_PARITYTYPE,
    LINE_CODING_DATABITS};

/* Abstract state of cdc device */
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
static uint8_t s_abstractState[COMM_FEATURE_DATA_SIZE] = {(STATUS_ABSTRACT_STATE >> 0U) & 0x00FFU,
                                                          (STATUS_ABSTRACT_STATE >> 8U) & 0x00FFU};

/* Country code of cdc device */
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
static uint8_t s_countryCode[COMM_FEATURE_DATA_SIZE] = {(COUNTRY_SETTING >> 0U) & 0x00FFU,
                                                        (COUNTRY_SETTING >> 8U) & 0x00FFU};

/* CDC ACM information */
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static usb_cdc_acm_info_t s_usbCdcAcmInfo;
/* Data buffer for receiving and sending*/
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static uint8_t s_currRecvBuf[DATA_BUFF_SIZE];
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static uint8_t s_currSendBuf[DATA_BUFF_SIZE];
volatile PL_UINT32 s_recvSize = 0;
volatile PL_UINT32 s_sendSize = 0;

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
//__attribute__((__section__(".ramfunc.$SRAMX")))
__attribute__((__section__(".ramfunc*")))
usb_status_t USB_DeviceCdcVcomCallback(class_handle_t handle, PL_UINT32 event, void *param)
{
    usb_status_t error = kStatus_USB_InvalidRequest;
    PL_UINT32 len;
    uint8_t *uartBitmap;
    usb_cdc_acm_info_t *acmInfo = &s_usbCdcAcmInfo;
    usb_device_cdc_acm_request_param_struct_t *acmReqParam;
    usb_device_endpoint_callback_message_struct_t *epCbParam;
    acmReqParam = (usb_device_cdc_acm_request_param_struct_t *)param;
    epCbParam   = (usb_device_endpoint_callback_message_struct_t *)param;
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
            else if ((1 == g_UsbCompositeDevPtr->cdcVcom.attach) && (1 == g_UsbCompositeDevPtr->cdcVcom.startTransactions))
            {
                if ((epCbParam->buffer != NULL) || ((epCbParam->buffer == NULL) && (epCbParam->length == 0)))
                {
                    /* User: add your own code for send complete event */
                    /* Schedule buffer for next receive event */
                    error = USB_DeviceCdcAcmRecv(handle, USB_CDC_VCOM_DIC_BULK_OUT_ENDPOINT, s_currRecvBuf,
                                                 g_cdcVcomDicEndpoints[1].maxPacketSize);
                }
            }
            else
            {
            }
        }
        break;
        case kUSB_DeviceCdcEventRecvResponse:
        {
            if ((1 == g_UsbCompositeDevPtr->cdcVcom.attach) && (1 == g_UsbCompositeDevPtr->cdcVcom.startTransactions))
            {
                s_recvSize = epCbParam->length;

                if (!s_recvSize)
                {
                    /* Schedule buffer for next receive event */
                    error = USB_DeviceCdcAcmRecv(handle, USB_CDC_VCOM_DIC_BULK_OUT_ENDPOINT, s_currRecvBuf,
                                                 g_cdcVcomDicEndpoints[1].maxPacketSize);
                }
				
				USB_DeviceCdcVcomTask_ConnectTuningTool();
            }
        }
        break;
        case kUSB_DeviceCdcEventSerialStateNotif:
            ((usb_device_cdc_acm_struct_t *)handle)->hasSentState = 0;
            error                                                 = kStatus_USB_Success;
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
                    *(acmReqParam->length) = sizeof(s_abstractState);
                }
                else
                {
                    /* no action, data phase, s_abstractState has been assigned */
                }
                error = kStatus_USB_Success;
            }
            else if (USB_DEVICE_CDC_FEATURE_COUNTRY_SETTING == acmReqParam->setupValue)
            {
                if (1 == acmReqParam->isSetup)
                {
                    *(acmReqParam->buffer) = s_countryCode;
                    *(acmReqParam->length) = sizeof(s_countryCode);
                }
                else
                {
                    /* no action, data phase, s_countryCode has been assigned */
                }
                error = kStatus_USB_Success;
            }
            else
            {
                /* no action, return kStatus_USB_InvalidRequest */
            }
            break;
        case kUSB_DeviceCdcEventGetCommFeature:
            if (USB_DEVICE_CDC_FEATURE_ABSTRACT_STATE == acmReqParam->setupValue)
            {
                *(acmReqParam->buffer) = s_abstractState;
                *(acmReqParam->length) = COMM_FEATURE_DATA_SIZE;
                error                  = kStatus_USB_Success;
            }
            else if (USB_DEVICE_CDC_FEATURE_COUNTRY_SETTING == acmReqParam->setupValue)
            {
                *(acmReqParam->buffer) = s_countryCode;
                *(acmReqParam->length) = COMM_FEATURE_DATA_SIZE;
                error                  = kStatus_USB_Success;
            }
            else
            {
                /* no action, return kStatus_USB_InvalidRequest */
            }
            break;
        case kUSB_DeviceCdcEventClearCommFeature:
            break;
        case kUSB_DeviceCdcEventGetLineCoding:
            *(acmReqParam->buffer) = s_lineCoding;
            *(acmReqParam->length) = LINE_CODING_SIZE;
            error                  = kStatus_USB_Success;
            break;
        case kUSB_DeviceCdcEventSetLineCoding:
        {
            if (1 == acmReqParam->isSetup)
            {
                *(acmReqParam->buffer) = s_lineCoding;
                *(acmReqParam->length) = sizeof(s_lineCoding);
            }
            else
            {
                /* no action, data phase, s_lineCoding has been assigned */
            }
            error = kStatus_USB_Success;
        }
        break;
        case kUSB_DeviceCdcEventSetControlLineState:
        {
            error                     = kStatus_USB_Success;
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
            uartBitmap    = (uint8_t *)&acmInfo->serialStateBuf[NOTIF_PACKET_SIZE + UART_BITMAP_SIZE - 2];
            uartBitmap[0] = acmInfo->uartState & 0xFFu;
            uartBitmap[1] = (acmInfo->uartState >> 8) & 0xFFu;
            len           = (PL_UINT32)(NOTIF_PACKET_SIZE + UART_BITMAP_SIZE);
            if (0 == ((usb_device_cdc_acm_struct_t *)handle)->hasSentState)
            {
                error =
                    USB_DeviceCdcAcmSend(handle, USB_CDC_VCOM_CIC_INTERRUPT_IN_ENDPOINT, acmInfo->serialStateBuf, len);
                if (kStatus_USB_Success != error)
                {
                    //usb_echo("kUSB_DeviceCdcEventSetControlLineState error!");
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
                if (1 == g_UsbCompositeDevPtr->cdcVcom.attach)
                {
                    g_UsbCompositeDevPtr->cdcVcom.startTransactions = 1;
                }
            }
            else
            {
                /* DTE_DEACTIVATED */
                if (1 == g_UsbCompositeDevPtr->cdcVcom.attach)
                {
                    g_UsbCompositeDevPtr->cdcVcom.startTransactions = 0;
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

#define MaxVComTransferPayloadSizeInByte	HS_CDC_VCOM_BULK_OUT_PACKET_SIZE
#define ComPacketHead_RdMcuMemory		0x11110000
#define ComPacketHead_WrMcuMemory		0x11110001
#define ComPacketHead_McuRdReturn		0x21110000
#define ComPacketHead_McuWrReturn		0x21110001

/*
//not useful, but it is useful in RT1060 prj
PL_UINT32 InvertU32(PL_UINT32 v)
{
	PL_UINT8 a,b,c,d;
	a=(v>> 0)&0xff;
	b=(v>> 8)&0xff;
	c=(v>>16)&0xff;
	d=(v>>24)&0xff;
	return(a*0x1000000 + b*0x10000 + c*0x100 + d);
}
*/

PL_UINT8 NeedToClearWhatIsReceivedFromComHostJustNow;
//__attribute__((__section__(".ramfunc.$SRAMX")))
__attribute__((__section__(".ramfunc*")))
void USB_DeviceCdcVcomTask_ConnectTuningTool(void)
{
	PL_UINT32 *p1;
	PL_UINT32 *p2;
	PL_UINT32 AuditU32;
	PL_UINT32 addr;
	PL_UINT32 WantedDataLenInBytes;
    usb_status_t error = kStatus_USB_Error;
	//PL_INT32 i;
    /* User Code */
    if ((0 != s_recvSize) && (0xFFFFFFFFU != s_recvSize))
    {
		PL_UINT32 Crc32Result;
		NeedToClearWhatIsReceivedFromComHostJustNow=1;
        p1=(PL_UINT32 *)s_currRecvBuf;
        if(*p1==ComPacketHead_RdMcuMemory)
        {	//read mcu memory
        	p1++;
        	addr=*p1++;
        	WantedDataLenInBytes=*p1++;
        	AuditU32=*p1++;
            CRC_WriteData(CRC_ENGINE, s_currRecvBuf, 12);
            Crc32Result = CRC_Get32bitResult(CRC_ENGINE);
            ResetCrc32Seed();
        	if(AuditU32==Crc32Result)
        	{	//check AuditU32 audit succ
        		if(WantedDataLenInBytes>MaxVComTransferPayloadSizeInByte) WantedDataLenInBytes=MaxVComTransferPayloadSizeInByte;
                p2=(PL_UINT32 *)s_currSendBuf;
                *p2++=ComPacketHead_McuRdReturn;
                *p2++=addr;
                *p2++=WantedDataLenInBytes;
                memcpy(p2,(PL_UINT8 *)addr,WantedDataLenInBytes);
                s_sendSize=16+WantedDataLenInBytes;
        	}else
        	{
    			//PRINTF("RT600: COM received data packet audit fail --- RdMcuMemory \r\n");
        	}
        }
        if(*p1==ComPacketHead_WrMcuMemory)
        {	//write mcu memory
        	p1++;
        	addr=*p1++;
        	WantedDataLenInBytes=*p1++;
            p1+=(WantedDataLenInBytes/4);
            AuditU32=*p1;
            CRC_WriteData(CRC_ENGINE, s_currRecvBuf, WantedDataLenInBytes+12);
            Crc32Result = CRC_Get32bitResult(CRC_ENGINE);
            ResetCrc32Seed();
          	if(AuditU32==Crc32Result)
            {	//check AuditU32 audit succ
        		if(WantedDataLenInBytes>MaxVComTransferPayloadSizeInByte) WantedDataLenInBytes=MaxVComTransferPayloadSizeInByte;
				p1=(PL_UINT32 *)s_currRecvBuf;
				p1+=3;
            	memcpy((PL_UINT8 *)addr,(PL_UINT8 *)p1,WantedDataLenInBytes);
                p2=(PL_UINT32 *)s_currSendBuf;
                *p2++=ComPacketHead_McuWrReturn;
                *p2++=addr;
                *p2++=WantedDataLenInBytes;
                s_sendSize=16;
            }else
            {
    			//PRINTF("RT600: COM received data packet audit fail --- WrMcuMemory \r\n");
            }
        }
        s_recvSize = 0;

    }else
    {
    	NeedToClearWhatIsReceivedFromComHostJustNow=0;
    }

    if (s_sendSize)
    {
        PL_UINT32 size = s_sendSize;
        s_sendSize = 0;
		PL_UINT32 Crc32Result;

		CRC_WriteData(CRC_ENGINE, s_currSendBuf, size-4);
		Crc32Result = CRC_Get32bitResult(CRC_ENGINE);
		ResetCrc32Seed();

		p2=(PL_UINT32 *)s_currSendBuf;
		p2+=(size-4)/4;
		*p2=Crc32Result;
        error = USB_DeviceCdcAcmSend(g_UsbCompositeDevPtr->cdcVcom.cdcAcmHandle, USB_CDC_VCOM_DIC_BULK_IN_ENDPOINT, s_currSendBuf, size);

        if (error != kStatus_USB_Success)
        {
            /* Failure to send Data Handling code here */
			//PRINTF("RT600: COM send bytes error --- RdMcuMemory \r\n");
        }

    }else
    {
    	if(NeedToClearWhatIsReceivedFromComHostJustNow)
    	{
			error = USB_DeviceCdcAcmSend(g_UsbCompositeDevPtr->cdcVcom.cdcAcmHandle, USB_CDC_VCOM_DIC_BULK_IN_ENDPOINT, s_currSendBuf, 0);
			NeedToClearWhatIsReceivedFromComHostJustNow=0;
    	}
    }
}

//__attribute__((__section__(".ramfunc.$SRAMX")))
__attribute__((__section__(".ramfunc*")))
void USB_DeviceCdcVcomTask(void)
{
	return;
}

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
        g_UsbCompositeDevPtr->cdcVcom.attach = 1;
        /* Schedule buffer for receive */
        USB_DeviceCdcAcmRecv(g_UsbCompositeDevPtr->cdcVcom.cdcAcmHandle, USB_CDC_VCOM_DIC_BULK_OUT_ENDPOINT, s_currRecvBuf,
                             g_cdcVcomDicEndpoints[1].maxPacketSize);
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
    return kStatus_USB_Success;
}


