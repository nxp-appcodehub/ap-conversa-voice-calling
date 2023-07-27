/*
 * Copyright 2020-2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include"usb_init.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

extern usb_device_endpoint_struct_t g_UsbDeviceAudioRecorderEndpoints[USB_AUDIO_RECORDER_STREAM_ENDPOINT_COUNT];
extern usb_device_endpoint_struct_t g_UsbDeviceAudioSpeakerEndpoints[USB_AUDIO_SPEAKER_STREAM_ENDPOINT_COUNT];
/*******************************************************************************
 * Code
 ******************************************************************************/

void USB_DeviceClockInit(void)
{
    uint32_t usbClockFreq = USB_CLOCK_FREQ;

    if (CONTROLLER_ID == kUSB_ControllerEhci0)
    {
        CLOCK_EnableUsbhs0PhyPllClock(kCLOCK_Usbphy480M, usbClockFreq);
        CLOCK_EnableUsbhs0Clock(kCLOCK_Usb480M, usbClockFreq);
    }
    else
    {
        CLOCK_EnableUsbhs1PhyPllClock(kCLOCK_Usbphy480M, usbClockFreq);
        CLOCK_EnableUsbhs1Clock(kCLOCK_Usb480M, usbClockFreq);
    }
    USB_EhciPhyInit(CONTROLLER_ID, BOARD_XTAL0_CLK_HZ, &g_phyConfig);
}


void USB_DeviceIsrEnable(void)
{
    uint8_t irqNumber;

    uint8_t usbDeviceEhciIrq[] = USBHS_IRQS;
    irqNumber                  = usbDeviceEhciIrq[CONTROLLER_ID - kUSB_ControllerEhci0];

    /* Install isr, set priority, and enable IRQ. */
    // NVIC_SetPriority((IRQn_Type)irqNumber, USB_DEVICE_INTERRUPT_PRIORITY);
    EnableIRQ((IRQn_Type)irqNumber);
}

status_t USB_Init(usb_device_composite_struct_t *p_usbCompositeStruct)
{
	usb_status_t retStatusUSB;
	status_t 	retStatus = kStatus_Success;
	PL_UINT8* 	p_UsbDeviceConfigurationDescriptor;
	PL_UINT16   indice;
	PL_UINT32   temp1_32b;

	/* Check input parameter and pointer null */
	if (p_usbCompositeStruct == PL_NULL)
	{
		return kStatus_NullPointer;
	}

	/*
	 * USB init clock
	 */
	USB_DeviceClockInit();
	/*
	 * USB update descriptor
     *     UPDATE TO SUPPORT USB TX RX DYNAMIC DESCRIPTOR
	 *     update only: Sample rate, channel number, bit width
     */

	p_usbCompositeStruct = &g_appHandle.usbTxRx_handle;
	p_UsbDeviceConfigurationDescriptor = &g_UsbDeviceConfigurationDescriptor[0];
	p_UsbDeviceConfigurationDescriptor[165]		 	 = p_usbCompositeStruct->USBTxchannelNumber;														// TX channel number
	p_UsbDeviceConfigurationDescriptor[175]			 = g_appHandle.audioDefinition.audioTxPath_handle.configParam.bitPerSample/8;						// TX bytedepth
	p_UsbDeviceConfigurationDescriptor[176]			 = g_appHandle.audioDefinition.audioTxPath_handle.configParam.bitPerSample;							// TX bitdepth
	p_UsbDeviceConfigurationDescriptor[181]			 = USB_SHORT_GET_LOW(  g_appHandle.usbTxRx_handle.audioUnified.iso_in_endp_packet_size 				// TX min packetSize
																		 + g_appHandle.audioDefinition.audioTxPath_handle.configParam.bitPerSample/8
																		 * g_appHandle.usbTxRx_handle.USBTxchannelNumber);
	p_UsbDeviceConfigurationDescriptor[182]			 = USB_SHORT_GET_HIGH( g_appHandle.usbTxRx_handle.audioUnified.iso_in_endp_packet_size				// TX max PacketSize
																		  +g_appHandle.audioDefinition.audioTxPath_handle.configParam.bitPerSample/8
																		  *g_appHandle.usbTxRx_handle.USBTxchannelNumber);
	p_UsbDeviceConfigurationDescriptor[220]		 	 = p_usbCompositeStruct->USBRxchannelNumber;														// RX channel number
	p_UsbDeviceConfigurationDescriptor[230]			 = g_appHandle.usbTxRx_handle.USBRxBitDepth/8;														// RX bytedepth
	p_UsbDeviceConfigurationDescriptor[231]			 = g_appHandle.usbTxRx_handle.USBRxBitDepth;														// RX bitdepth
	p_UsbDeviceConfigurationDescriptor[236]			 = USB_SHORT_GET_LOW(g_appHandle.usbTxRx_handle.audioUnified.iso_out_endp_packet_size);				//RX min packetSize
	p_UsbDeviceConfigurationDescriptor[237]			 = USB_SHORT_GET_HIGH(g_appHandle.usbTxRx_handle.audioUnified.iso_out_endp_packet_size);			//RX max PacketSize

	/*
	 * USB init parameters
	 */
    p_usbCompositeStruct->speed                            			= USB_SPEED_HIGH;   // SPEED_HERE
    p_usbCompositeStruct->attach                           			= 0U;
    p_usbCompositeStruct->audioUnified.audioSpeakerHandle 			= (class_handle_t) NULL;
    p_usbCompositeStruct->audioUnified.audioRecorderHandle 			= (class_handle_t) NULL;
    p_usbCompositeStruct->cdcVcom.cdcAcmHandle 						= (class_handle_t) NULL;
    p_usbCompositeStruct->deviceHandle                     			= NULL;

    retStatus  														= USB_DeviceClassInit(CONTROLLER_ID, &g_UsbDeviceCompositeConfigList, &p_usbCompositeStruct->deviceHandle);
    if (kStatus_USB_Success != retStatus)
    {
        usb_echo("USB device composite demo init failed\r\n");
        return kStatus_Fail;
    }
    else
    {

    	g_UsbDeviceAudioRecorderEndpoints->maxPacketSize 			= p_usbCompositeStruct->audioUnified.iso_in_endp_packet_size  + g_appHandle.audioDefinition.audioTxPath_handle.configParam.bitPerSample/8 * p_usbCompositeStruct->USBTxchannelNumber;
    	g_UsbDeviceAudioSpeakerEndpoints->maxPacketSize 			= p_usbCompositeStruct->audioUnified.iso_out_endp_packet_size;
    	p_usbCompositeStruct->cdcVcom.cdcAcmHandle 				 	= g_UsbDeviceCompositeConfigList.config[0].classHandle;
    	p_usbCompositeStruct->audioUnified.audioRecorderHandle 		= g_UsbDeviceCompositeConfigList.config[1].classHandle;
    	p_usbCompositeStruct->audioUnified.audioSpeakerHandle  		= g_UsbDeviceCompositeConfigList.config[2].classHandle;
		USB_DeviceAudioCompositeInit(p_usbCompositeStruct);
		USB_DeviceCdcVcomInit(p_usbCompositeStruct);
    }

	/*
	 * USB apply descriptor
	 *   after that the USB works with interrupts
	 */
    USB_DeviceIsrEnable();
    /*Add one delay here to make the DP pull down long enough to allow host to detect the previous disconnection.*/
    SDK_DelayAtLeastUs(5000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
    USB_DeviceRun(p_usbCompositeStruct->deviceHandle);
}
