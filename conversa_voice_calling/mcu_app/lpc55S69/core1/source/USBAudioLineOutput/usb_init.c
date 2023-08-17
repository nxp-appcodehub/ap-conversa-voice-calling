
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

//fsl driver headers
#include "fsl_ctimer.h"
#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
	#include "fsl_sysmpu.h"
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */

#if ((defined FSL_FEATURE_SOC_USBPHY_COUNT) && (FSL_FEATURE_SOC_USBPHY_COUNT > 0U))
	#include "usb_phy.h"
#endif

//Application headers
#include "appGlobal.h"
#include "usb_init.h"





/*******************************************************************************
 * Macro Definitions
 ******************************************************************************/


/*******************************************************************************
 * Prototypes
 ******************************************************************************/



extern PL_UINT32 USB_AudioSpeakerBufferSpaceUsed(void);
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
	#if (defined USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS)
		void CTIMER_SOF_TOGGLE_HANDLER_FRO(PL_UINT32 i);
	#endif
	void CTIMER_SOF_TOGGLE_HANDLER_PLL(PL_UINT32 i);
#else
	extern void USB_DeviceCalculateFeedback(void);
#endif
void USB_DeviceClockInit(void);
void USB_DeviceIsrEnable(void);
#if USB_DEVICE_CONFIG_USE_TASK
	void USB_DeviceTaskFn(void *deviceHandle);
#endif
extern void USB_AudioCodecTask(void);
extern void USB_AudioSpeakerResetTask(void);
extern void USB_DeviceAudioSpeakerStatusReset(void);


/* Composite device structure. */
usb_device_composite_struct_t *g_UsbCompositeDevPtr;


usb_status_t USB_DeviceCallback(usb_device_handle handle, PL_UINT32 event, void *param);

#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
	#if (defined USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS)
		static ctimer_config_t ctimerInfoFro;
		ctimer_callback_t *cb_func_fro[] = {(ctimer_callback_t *)CTIMER_SOF_TOGGLE_HANDLER_FRO};
	#endif
	ctimer_callback_t *cb_func_pll[] = {(ctimer_callback_t *)CTIMER_SOF_TOGGLE_HANDLER_PLL};
	static ctimer_config_t ctimerInfoPll;
#endif




#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
void USB_AudioPllChange(void)
{
    SYSCON->PLL0SSCG0 = g_appHandle.UsbCompositeDev_handle.audioUnified.curAudioPllFrac;
}

void CTIMER_CaptureInit(void)
{
	#if (defined USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS)
		INPUTMUX->TIMER0CAPTSEL[0] = 0x14U; /* 0x15U for USB1 and 0x14U for USB0. */
	#elif (defined USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS)
		INPUTMUX->TIMER0CAPTSEL[0] = 0x15U; /* 0x15U for USB1 and 0x14U for USB0. */
	#endif
    CTIMER_GetDefaultConfig(&ctimerInfoPll);

    /* Initialize CTimer module */
    CTIMER_Init(CTIMER0, &ctimerInfoPll);

    CTIMER_SetupCapture(CTIMER0, kCTIMER_Capture_0, kCTIMER_Capture_RiseEdge, true);

    CTIMER_RegisterCallBack(CTIMER0, (ctimer_callback_t *)&cb_func_pll[0], kCTIMER_SingleCallback);

    /* Start the L counter */
    CTIMER_StartTimer(CTIMER0);

    /* if full speed controller, use another ctimer */
	#if (defined USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS)
		INPUTMUX->TIMER1CAPTSEL[0] = 0x14U; /* 0x15U for USB1 and 0x14U for USB0. */

		CTIMER_GetDefaultConfig(&ctimerInfoFro);

		CTIMER_Init(CTIMER1, &ctimerInfoFro);

		CTIMER_SetupCapture(CTIMER1, kCTIMER_Capture_0, kCTIMER_Capture_RiseEdge, true);
		/* Receive notification when event is triggered */
		CTIMER_RegisterCallBack(CTIMER1, (ctimer_callback_t *)&cb_func_fro[0], kCTIMER_SingleCallback);

		/* Start the L counter */
		CTIMER_StartTimer(CTIMER1);
	#endif
}
#endif



extern usb_device_class_struct_t g_UsbDeviceCdcVcomConfig;
extern usb_device_class_struct_t g_UsbDeviceAudioClassRecorder;
extern usb_device_class_struct_t g_UsbDeviceAudioClassSpeaker;
extern usb_device_composite_struct_t *g_UsbDeviceComposite;
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
#else
	extern uint8_t audioFeedBackBuffer[4];
#endif


/* USB device class information */
static usb_device_class_config_struct_t g_CompositeClassConfig[3] =
{
	{
		USB_DeviceAudioCompositeCallback,
		(class_handle_t)NULL,
		&g_UsbDeviceAudioClassRecorder,
	},
	{
		USB_DeviceAudioCompositeCallback,
		(class_handle_t)NULL,
		&g_UsbDeviceAudioClassSpeaker,
	},
	{
		USB_DeviceCdcVcomCallback,
		(class_handle_t)NULL,
		&g_UsbDeviceCdcVcomConfig,
	}
};

/* USB device class configuration information */
static usb_device_class_config_list_struct_t g_UsbDeviceCompositeConfigList =
{
    g_CompositeClassConfig,
    USB_DeviceCallback,
    3
};





/*******************************************************************************
 * Code
 ******************************************************************************/



#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
__attribute__((__section__(".ramfunc*")))
void audio_fro_trim_up(void)
{
    static uint8_t usbClkAdj = 0;
    if (0U == usbClkAdj)
    {
        /* USBCLKADJ is turned off, start using software adjustment */
        ANACTRL->FRO192M_CTRL = (ANACTRL->FRO192M_CTRL & ~(ANACTRL_FRO192M_CTRL_USBCLKADJ_MASK));
        usbClkAdj             = 1U;
    }
    PL_UINT32 val          = ANACTRL->FRO192M_CTRL;
    val                   = (val & ~(0xff << 16)) | ((((val >> 16) & 0xFF) + 1) << 16) | (1UL << 31);
    ANACTRL->FRO192M_CTRL = val;
}

__attribute__((__section__(".ramfunc*")))
void audio_fro_trim_down(void)
{
    static uint8_t usbClkAdj = 0;
    if (0U == usbClkAdj)
    {
        /* USBCLKADJ is turned off, start using software adjustment */
        ANACTRL->FRO192M_CTRL = (ANACTRL->FRO192M_CTRL & ~(ANACTRL_FRO192M_CTRL_USBCLKADJ_MASK));
        usbClkAdj             = 1U;
    }
    PL_UINT32 val          = ANACTRL->FRO192M_CTRL;
    ANACTRL->FRO192M_CTRL = (val & ~(0xff << 16)) | ((((val >> 16) & 0xFF) - 1) << 16) | (1UL << 31);
}
#endif






#if (defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U))
__attribute__((__section__(".ramfunc*")))
void USB0_IRQHandler(void)
{
    USB_DeviceLpcIp3511IsrFunction(g_appHandle.UsbCompositeDev_handle.deviceHandle);
}
#endif
#if (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
void USB1_IRQHandler(void)
{
    USB_DeviceLpcIp3511IsrFunction(g_appHandle.UsbCompositeDev_handle.deviceHandle);
}
#endif
void USB_DeviceClockInit(void)
{
	#if defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U)
		usb_phy_config_struct_t phyConfig = {
			BOARD_USB_PHY_D_CAL,
			BOARD_USB_PHY_TXCAL45DP,
			BOARD_USB_PHY_TXCAL45DM,
		};
	#endif

	#if defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
		/* enable USB IP clock */
		CLOCK_EnableUsbfs0DeviceClock(kCLOCK_UsbfsSrcFro, CLOCK_GetFroHfFreq());
		ANACTRL->FRO192M_CTRL = (ANACTRL->FRO192M_CTRL & ~(ANACTRL_FRO192M_CTRL_USBCLKADJ_MASK));
		#if defined(FSL_FEATURE_USB_USB_RAM) && (FSL_FEATURE_USB_USB_RAM)
			for (PL_INT32 i = 0; i < FSL_FEATURE_USB_USB_RAM; i++)
			{
				((uint8_t *)FSL_FEATURE_USB_USB_RAM_BASE_ADDRESS)[i] = 0x00U;
			}
		#endif
	#endif

	#if defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U)
		/* enable USB IP clock */
		CLOCK_EnableUsbhs0PhyPllClock(kCLOCK_UsbPhySrcExt, BOARD_XTAL0_CLK_HZ);
		CLOCK_EnableUsbhs0DeviceClock(kCLOCK_UsbSrcUnused, 0U);
		USB_EhciPhyInit(CONTROLLER_ID, BOARD_XTAL0_CLK_HZ, &phyConfig);
		#if defined(FSL_FEATURE_USBHSD_USB_RAM) && (FSL_FEATURE_USBHSD_USB_RAM)
			for (PL_INT32 i = 0; i < FSL_FEATURE_USBHSD_USB_RAM; i++)
			{
				((uint8_t *)FSL_FEATURE_USBHSD_USB_RAM_BASE_ADDRESS)[i] = 0x00U;
			}
		#endif
	#endif
}
void USB_DeviceIsrEnable(void)
{
    uint8_t irqNumber;
	#if defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
		uint8_t usbDeviceIP3511Irq[] = USB_IRQS;
		irqNumber                    = usbDeviceIP3511Irq[CONTROLLER_ID - kUSB_ControllerLpcIp3511Fs0];
	#endif
	#if defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U)
		uint8_t usbDeviceIP3511Irq[] = USBHSD_IRQS;
		irqNumber                    = usbDeviceIP3511Irq[CONTROLLER_ID - kUSB_ControllerLpcIp3511Hs0];
	#endif
    /* Install isr, set priority, and enable IRQ. */
    NVIC_SetPriority((IRQn_Type)irqNumber, IRQ_PRIORITY_USB);
    EnableIRQ((IRQn_Type)irqNumber);
}

void USB_DeviceIsrDisable(void)
{
    uint8_t irqNumber;
	#if defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
		uint8_t usbDeviceIP3511Irq[] = USB_IRQS;
		irqNumber                    = usbDeviceIP3511Irq[CONTROLLER_ID - kUSB_ControllerLpcIp3511Fs0];
	#endif
	#if defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U)
		uint8_t usbDeviceIP3511Irq[] = USBHSD_IRQS;
		irqNumber                    = usbDeviceIP3511Irq[CONTROLLER_ID - kUSB_ControllerLpcIp3511Hs0];
	#endif
    DisableIRQ((IRQn_Type)irqNumber);
}

#if USB_DEVICE_CONFIG_USE_TASK
void USB_DeviceTaskFn(void *deviceHandle)
{
	#if defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
		USB_DeviceLpcIp3511TaskFunction(deviceHandle);
	#endif
	#if defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U)
		USB_DeviceLpcIp3511TaskFunction(deviceHandle);
	#endif
}
#endif

/*!
 * @brief USB device callback function.
 *
 * This function handles the usb device specific requests.
 *
 * @param handle		  The USB device handle.
 * @param event 		  The USB device event type.
 * @param param 		  The parameter of the device specific request.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
__attribute__((__section__(".ramfunc*")))
usb_status_t USB_DeviceCallback(usb_device_handle handle, PL_UINT32 event, void *param)
{
    usb_status_t error = kStatus_USB_InvalidRequest;
    uint16_t *temp16   = (uint16_t *)param;
    uint8_t *temp8     = (uint8_t *)param;
    uint8_t count      = 0U;

    switch (event)
    {
        case kUSB_DeviceEventBusReset:
        {
            for (count = 0U; count < USB_DEVICE_INTERFACE_COUNT; count++)
            {
                g_appHandle.UsbCompositeDev_handle.currentInterfaceAlternateSetting[count] = 0U;
            }
            /* reset audio speaker status to be the initialized status */
            USB_DeviceAudioSpeakerStatusReset();
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
#else
            /* reset the the last feedback value */
            g_appHandle.UsbCompositeDev_handle.audioUnified.lastFeedbackValue             = 0U;
#endif
            g_appHandle.UsbCompositeDev_handle.attach               = 0U;
            g_appHandle.UsbCompositeDev_handle.currentConfiguration = 0U;
            error                            = kStatus_USB_Success;
#if (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)) || (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
            /* Get USB speed to configure the device, including max packet size and interval of the endpoints. */
            if (kStatus_USB_Success == USB_DeviceClassGetSpeed(CONTROLLER_ID, &g_appHandle.UsbCompositeDev_handle.speed))
            {
                USB_DeviceSetSpeed(handle, g_appHandle.UsbCompositeDev_handle.speed);
            }
            if (USB_SPEED_HIGH == g_appHandle.UsbCompositeDev_handle.speed)
            {
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
                g_appHandle.UsbCompositeDev_handle.audioUnified.currentStreamOutMaxPacketSize = (g_appHandle.UsbCompositeDev_handle.HS_ISO_OUT_ENDP_PACKET_SIZE);
#else
                g_appHandle.UsbCompositeDev_handle.audioUnified.currentStreamOutMaxPacketSize =
                    (HS_ISO_OUT_ENDP_PACKET_SIZE + AUDIO_OUT_FORMAT_CHANNELS * AUDIO_OUT_FORMAT_SIZE);
                g_appHandle.UsbCompositeDev_handle.audioUnified.currentFeedbackMaxPacketSize = HS_ISO_FEEDBACK_ENDP_PACKET_SIZE;
#endif /* USB_DEVICE_AUDIO_USE_SYNC_MODE */
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
                /* high speed and audio 2.0, audio play interval is set by HS EP packet size */
                g_appHandle.UsbCompositeDev_handle.audioUnified.audioPlayTransferSize = g_appHandle.UsbCompositeDev_handle.HS_ISO_OUT_ENDP_PACKET_SIZE;
                /* use short play buffer size, only use two elements */
                g_appHandle.UsbCompositeDev_handle.audioUnified.audioPlayBufferSize =
                		g_appHandle.UsbCompositeDev_handle.AUDIO_OUT_TRANSFER_LENGTH_ONE_FRAME * AUDIO_SPEAKER_DATA_WHOLE_BUFFER_COUNT;
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
#else
#if defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U)
                AUDIO_UPDATE_FEEDBACK_DATA(audioFeedBackBuffer, AUDIO_SAMPLING_RATE_TO_16_16_SPECIFIC);
#else
                AUDIO_UPDATE_FEEDBACK_DATA(audioFeedBackBuffer, AUDIO_SAMPLING_RATE_TO_16_16);
#endif
#endif
#else
                /* high speed and audio 1.0, audio play interval is 1 ms using this play size */
                g_appHandle.UsbCompositeDev_handle.audioUnified.audioPlayTransferSize = AUDIO_OUT_TRANSFER_LENGTH_ONE_FRAME;
                /* use the whole play buffer size */
                g_appHandle.UsbCompositeDev_handle.audioUnified.audioPlayBufferSize =
                    AUDIO_SPEAKER_DATA_WHOLE_BUFFER_COUNT_NORMAL * AUDIO_OUT_TRANSFER_LENGTH_ONE_FRAME;
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
#else
                AUDIO_UPDATE_FEEDBACK_DATA(audioFeedBackBuffer, AUDIO_SAMPLING_RATE_TO_10_14);
#endif
#endif /* USB_DEVICE_CONFIG_AUDIO_CLASS_2_0 */
                g_UsbCompositeDevPtr->audioUnified.speed = USB_SPEED_HIGH;
            }
            else
            {
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
                g_appHandle.UsbCompositeDev_handle.audioUnified.currentStreamOutMaxPacketSize = (g_appHandle.UsbCompositeDev_handle.FS_ISO_OUT_ENDP_PACKET_SIZE);
#else
                g_appHandle.UsbCompositeDev_handle.audioUnified.currentStreamOutMaxPacketSize =
                    (FS_ISO_OUT_ENDP_PACKET_SIZE + AUDIO_OUT_FORMAT_CHANNELS * AUDIO_OUT_FORMAT_SIZE);
                g_appHandle.UsbCompositeDev_handle.audioUnified.currentFeedbackMaxPacketSize = FS_ISO_FEEDBACK_ENDP_PACKET_SIZE;
                AUDIO_UPDATE_FEEDBACK_DATA(audioFeedBackBuffer, AUDIO_SAMPLING_RATE_TO_10_14);
#endif
                /* full speed, audio play interval is 1 ms using this play size */
                g_appHandle.UsbCompositeDev_handle.audioUnified.audioPlayTransferSize = g_appHandle.UsbCompositeDev_handle.AUDIO_OUT_TRANSFER_LENGTH_ONE_FRAME;
                /* use the whole play buffer size */
                g_appHandle.UsbCompositeDev_handle.audioUnified.audioPlayBufferSize =
                    AUDIO_SPEAKER_DATA_WHOLE_BUFFER_COUNT_NORMAL * g_appHandle.UsbCompositeDev_handle.AUDIO_OUT_TRANSFER_LENGTH_ONE_FRAME;
            }
#else
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
            g_appHandle.UsbCompositeDev_handle.audioUnified.currentStreamOutMaxPacketSize = (FS_ISO_OUT_ENDP_PACKET_SIZE);
#else
            g_appHandle.UsbCompositeDev_handle.audioUnified.currentStreamOutMaxPacketSize =
                (FS_ISO_OUT_ENDP_PACKET_SIZE + AUDIO_OUT_FORMAT_CHANNELS * AUDIO_OUT_FORMAT_SIZE);
            g_appHandle.UsbCompositeDev_handle.audioUnified.currentFeedbackMaxPacketSize = FS_ISO_FEEDBACK_ENDP_PACKET_SIZE;
            AUDIO_UPDATE_FEEDBACK_DATA(audioFeedBackBuffer, AUDIO_SAMPLING_RATE_TO_10_14);
            /* reset the the last feedback value */
            g_UsbCompositeDevPtr->audioUnified.lastFeedbackValue = 0U;
#endif
            /* full speed, audio play interval is 1 ms using this play size */
            g_appHandle.UsbCompositeDev_handle.audioUnified.audioPlayTransferSize = AUDIO_OUT_TRANSFER_LENGTH_ONE_FRAME;
            /* use the whole play buffer size */
            g_appHandle.UsbCompositeDev_handle.audioUnified.audioPlayBufferSize =
                AUDIO_SPEAKER_DATA_WHOLE_BUFFER_COUNT_NORMAL * AUDIO_OUT_TRANSFER_LENGTH_ONE_FRAME;
#endif /* USB_DEVICE_CONFIG_EHCI, USB_DEVICE_CONFIG_LPCIP3511HS */
        }
        break;
        case kUSB_DeviceEventSetConfiguration:
            if (0U == (*temp8))
            {
                g_appHandle.UsbCompositeDev_handle.attach               = 0U;
                g_appHandle.UsbCompositeDev_handle.currentConfiguration = 0U;
                error                            = kStatus_USB_Success;
            }
            else if (USB_COMPOSITE_CONFIGURE_INDEX == (*temp8))
            {
                g_appHandle.UsbCompositeDev_handle.attach               = 1U;
                g_appHandle.UsbCompositeDev_handle.currentConfiguration = *temp8;
                USB_DeviceAudioCompositeSetConfigure(g_appHandle.UsbCompositeDev_handle.audioUnified.audioSpeakerHandle,  *temp8);
                USB_DeviceAudioCompositeSetConfigure(g_appHandle.UsbCompositeDev_handle.audioUnified.audioRecorderHandle, *temp8);
                USB_DeviceCdcVcomSetConfigure       (g_appHandle.UsbCompositeDev_handle.cdcVcom.cdcAcmHandle,             *temp8);

                error = kStatus_USB_Success;
            }
            else
            {
            }
            break;
        case kUSB_DeviceEventSetInterface:

            if (g_appHandle.UsbCompositeDev_handle.attach)
            {
                uint8_t interface        = (uint8_t)((*temp16 & 0xFF00U) >> 0x08U);
                uint8_t alternateSetting = (uint8_t)(*temp16 & 0x00FFU);

                switch (interface)
                {
					case USB_CDC_VCOM_CIC_INTERFACE_INDEX:
	                    if (alternateSetting < USB_CDC_VCOM_CIC_INTERFACE_ALTERNATE_COUNT)
	                    {
	                        g_appHandle.UsbCompositeDev_handle.currentInterfaceAlternateSetting[interface] = alternateSetting;
	                        error                                                   = kStatus_USB_Success;
	                    }
						break;
					case USB_CDC_VCOM_DIC_INTERFACE_INDEX:
	                    if (alternateSetting < USB_CDC_VCOM_DIC_INTERFACE_ALTERNATE_COUNT)
	                    {
	                        g_appHandle.UsbCompositeDev_handle.currentInterfaceAlternateSetting[interface] = alternateSetting;
	                        error                                                   = kStatus_USB_Success;
	                    }
						break;
                    case USB_AUDIO_CONTROL_INTERFACE_INDEX:
                        if (alternateSetting < USB_AUDIO_CONTROL_INTERFACE_ALTERNATE_COUNT)
                        {
                            error = kStatus_USB_Success;
                        }
                        break;
                    case USB_AUDIO_RECORDER_STREAM_INTERFACE_INDEX:
                        if (alternateSetting < USB_AUDIO_RECORDER_STREAM_INTERFACE_ALTERNATE_COUNT)
                        {
                            error = USB_DeviceAudioRecorderSetInterface(g_appHandle.UsbCompositeDev_handle.audioUnified.audioRecorderHandle,
                                                                        interface, alternateSetting);
                        }
                        break;
                    case USB_AUDIO_SPEAKER_STREAM_INTERFACE_INDEX:
                        if (alternateSetting < USB_AUDIO_SPEAKER_STREAM_INTERFACE_ALTERNATE_COUNT)
                        {
                            error = USB_DeviceAudioSpeakerSetInterface(g_appHandle.UsbCompositeDev_handle.audioUnified.audioSpeakerHandle,
                                                                       interface, alternateSetting);
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
                            if (USB_AUDIO_SPEAKER_STREAM_INTERFACE_ALTERNATE_1 == alternateSetting)
                            {
                                g_appHandle.UsbCompositeDev_handle.audioUnified.stopDataLengthAudioAdjust = 0U;
                            }
                            else if (USB_AUDIO_SPEAKER_STREAM_INTERFACE_ALTERNATE_0 == alternateSetting)
                            {
                                g_appHandle.UsbCompositeDev_handle.audioUnified.stopDataLengthAudioAdjust = 1U;
                            }
#else
                            /* usb host stops the speaker, so there is no need for feedback */
                            if ((1U == g_appHandle.UsbCompositeDev_handle.audioUnified.startPlayFlag) &&
                                (USB_AUDIO_SPEAKER_STREAM_INTERFACE_ALTERNATE_0 == alternateSetting))
                            {
                                g_appHandle.UsbCompositeDev_handle.audioUnified.stopFeedbackUpdate = 1U;
                            }

                            /* usb host start the speaker, discard the feedback for AUDIO_SPEAKER_FEEDBACK_DISCARD_COUNT
                             * times */
                            if (USB_AUDIO_SPEAKER_STREAM_INTERFACE_ALTERNATE_1 == alternateSetting)
                            {
                                g_appHandle.UsbCompositeDev_handle.audioUnified.stopFeedbackUpdate              = 0U;
                                g_UsbCompositeDevPtr->audioUnified.feedbackDiscardFlag = 1U;
                                g_UsbCompositeDevPtr->audioUnified.feedbackDiscardTimes =
                                    AUDIO_SPEAKER_FEEDBACK_DISCARD_COUNT;
                            }
#endif
                        }
                        break;
                    default:
                        break;
                }

                if (kStatus_USB_Success == error)
                {
                    g_appHandle.UsbCompositeDev_handle.currentInterfaceAlternateSetting[interface] = alternateSetting;
                }
            }
            break;
        case kUSB_DeviceEventGetConfiguration:
            if (param)
            {
                *temp8 = g_appHandle.UsbCompositeDev_handle.currentConfiguration;
                error  = kStatus_USB_Success;
            }
            break;
        case kUSB_DeviceEventGetInterface:
            if (param)
            {
                uint8_t interface = (uint8_t)((*temp16 & 0xFF00U) >> 0x08U);
                if (interface < USB_DEVICE_INTERFACE_COUNT)
                {
                    *temp16 = (*temp16 & 0xFF00U) | g_appHandle.UsbCompositeDev_handle.currentInterfaceAlternateSetting[interface];
                    error   = kStatus_USB_Success;
                }
            }
            break;
        case kUSB_DeviceEventGetDeviceDescriptor:
            if (param)
            {
                error = USB_DeviceGetDeviceDescriptor(handle, (usb_device_get_device_descriptor_struct_t *)param);
            }
            break;
        case kUSB_DeviceEventGetConfigurationDescriptor:
            if (param)
            {
                error = USB_DeviceGetConfigurationDescriptor(handle,
                                                             (usb_device_get_configuration_descriptor_struct_t *)param);
            }
            break;
        case kUSB_DeviceEventGetStringDescriptor:
            if (param)
            {
                error = USB_DeviceGetStringDescriptor(handle, (usb_device_get_string_descriptor_struct_t *)param);
            }
            break;
#if (defined(USB_DEVICE_CONFIG_CV_TEST) && (USB_DEVICE_CONFIG_CV_TEST > 0U))
        case kUSB_DeviceEventGetDeviceQualifierDescriptor:
            if (param)
            {
                /* Get device descriptor request */
                error = USB_DeviceGetDeviceQualifierDescriptor(
                    handle, (usb_device_get_device_qualifier_descriptor_struct_t *)param);
            }
            break;
#endif
        default:
            break;
    }

    return error;
}

/*!
 * @brief Application initialization function.
 *
 * This function initializes the application.
 *
 * @return None.
 */
void InitUsbCompositeDevice(void)
{
	//set default values of the UAC interface values

	if(g_appHandle.audioDefinition.audioRxPath_handle.configParam.source == AUDIO_SRC_USB1)
	{
		g_appHandle.UsbCompositeDev_handle.AUDIO_OUT_FORMAT_CHANNELS=1;									// number of usb RX channel number			1 (can be 1 or 2)
	}else
		if(g_appHandle.audioDefinition.audioRxPath_handle.configParam.source == AUDIO_SRC_USB2)
		{
			g_appHandle.UsbCompositeDev_handle.AUDIO_OUT_FORMAT_CHANNELS=2;								// number of usb RX channel number			2
		}else
		{
			g_appHandle.UsbCompositeDev_handle.AUDIO_OUT_FORMAT_CHANNELS=2;								// number of usb RX channel number			2
		}


	if(g_appHandle.audioDefinition.audioTxPath_handle.configParam.sink == AUDIO_SINK_USB1)
	{
		g_appHandle.UsbCompositeDev_handle.AUDIO_IN_FORMAT_CHANNELS=1;									// number of usb TX channel number			1 ( can be 1,2,3,4  --- all tested OK)
	}else
		if(g_appHandle.audioDefinition.audioTxPath_handle.configParam.sink == AUDIO_SINK_USB4)
		{
			g_appHandle.UsbCompositeDev_handle.AUDIO_IN_FORMAT_CHANNELS=4;								// number of usb TX channel number			4 ( can be 1,2,3,4  --- all tested OK)
		}else
		{
			g_appHandle.UsbCompositeDev_handle.AUDIO_IN_FORMAT_CHANNELS=4;								// number of usb TX channel number			4 ( can be 1,2,3,4  --- all tested OK)
		}

	if(g_appHandle.audioDefinition.audioRxPath_handle.configParam.bitPerSample==16)
	{
		g_appHandle.UsbCompositeDev_handle.AUDIO_OUT_FORMAT_SIZE=2;										// number of bits per sample in USBRX flow    2 or 4 (bytes)
	}else
	{
		g_appHandle.UsbCompositeDev_handle.AUDIO_OUT_FORMAT_SIZE=4;										// number of bits per sample in USBRX flow    2 or 4 (bytes)
	}

	if(g_appHandle.audioDefinition.audioTxPath_handle.configParam.bitPerSample==16)
	{
		g_appHandle.UsbCompositeDev_handle.AUDIO_IN_FORMAT_SIZE=2;										// number of bits per sample in USBTX flow    2 or 4 (bytes)
	}else
	{
		g_appHandle.UsbCompositeDev_handle.AUDIO_IN_FORMAT_SIZE=4;										// number of bits per sample in USBTX flow    2 or 4 (bytes)
	}
	if (  (g_appHandle.audioDefinition.audioTxPath_handle.configParam.sampleRate == 16000)
		&&(g_appHandle.audioDefinition.audioRxPath_handle.configParam.sampleRate == 16000))
	{
	g_appHandle.UsbCompositeDev_handle.AUDIO_IN_SAMPLING_RATE_KHZ=16;
	g_appHandle.UsbCompositeDev_handle.AUDIO_OUT_SAMPLING_RATE_KHZ=16;
	}
	else if (  (g_appHandle.audioDefinition.audioTxPath_handle.configParam.sampleRate == 32000)
			&&(g_appHandle.audioDefinition.audioRxPath_handle.configParam.sampleRate == 32000))
	{
		g_appHandle.UsbCompositeDev_handle.AUDIO_IN_SAMPLING_RATE_KHZ=32;
		g_appHandle.UsbCompositeDev_handle.AUDIO_OUT_SAMPLING_RATE_KHZ=32;
	}

	g_appHandle.UsbCompositeDev_handle.AUDIO_OUT_TRANSFER_LENGTH_ONE_FRAME	=g_appHandle.UsbCompositeDev_handle.AUDIO_OUT_SAMPLING_RATE_KHZ * g_appHandle.UsbCompositeDev_handle.AUDIO_OUT_FORMAT_CHANNELS * g_appHandle.UsbCompositeDev_handle.AUDIO_OUT_FORMAT_SIZE;
	g_appHandle.UsbCompositeDev_handle.FS_ISO_OUT_ENDP_PACKET_SIZE			=g_appHandle.UsbCompositeDev_handle.AUDIO_OUT_TRANSFER_LENGTH_ONE_FRAME;
	g_appHandle.UsbCompositeDev_handle.FS_ISO_IN_ENDP_PACKET_SIZE			=g_appHandle.UsbCompositeDev_handle.AUDIO_IN_SAMPLING_RATE_KHZ * g_appHandle.UsbCompositeDev_handle.AUDIO_IN_FORMAT_CHANNELS * g_appHandle.UsbCompositeDev_handle.AUDIO_IN_FORMAT_SIZE;
	g_appHandle.UsbCompositeDev_handle.HS_ISO_OUT_ENDP_PACKET_SIZE			=g_appHandle.UsbCompositeDev_handle.AUDIO_OUT_TRANSFER_LENGTH_ONE_FRAME;
	g_appHandle.UsbCompositeDev_handle.HS_ISO_IN_ENDP_PACKET_SIZE			=g_appHandle.UsbCompositeDev_handle.AUDIO_IN_SAMPLING_RATE_KHZ * g_appHandle.UsbCompositeDev_handle.AUDIO_IN_FORMAT_CHANNELS * g_appHandle.UsbCompositeDev_handle.AUDIO_IN_FORMAT_SIZE;

	g_UsbCompositeDevPtr=&g_appHandle.UsbCompositeDev_handle;
	FillUsbDescriptorRelatedValuesBeforeUsbInit();
    USB_DeviceClockInit();
#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
    SYSMPU_Enable(SYSMPU, 0);
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */

    g_appHandle.UsbCompositeDev_handle.speed                            = USB_SPEED_FULL;
    g_appHandle.UsbCompositeDev_handle.attach                           = 0U;
    g_appHandle.UsbCompositeDev_handle.audioUnified.audioSpeakerHandle  = (class_handle_t)NULL;
    g_appHandle.UsbCompositeDev_handle.audioUnified.audioRecorderHandle = (class_handle_t)NULL;
    g_appHandle.UsbCompositeDev_handle.cdcVcom.cdcAcmHandle = 			   (class_handle_t)NULL;
    g_appHandle.UsbCompositeDev_handle.deviceHandle                     = NULL;

    if (kStatus_USB_Success !=
        USB_DeviceClassInit(CONTROLLER_ID, &g_UsbDeviceCompositeConfigList, &g_appHandle.UsbCompositeDev_handle.deviceHandle))
    {
        return;
    }
    else
    {
        usb_echo("core1: USB device init\r\n");

		g_appHandle.UsbCompositeDev_handle.audioUnified.audioRecorderHandle = g_UsbDeviceCompositeConfigList.config[0].classHandle;
		g_appHandle.UsbCompositeDev_handle.audioUnified.audioSpeakerHandle  = g_UsbDeviceCompositeConfigList.config[1].classHandle;
		g_appHandle.UsbCompositeDev_handle.cdcVcom.cdcAcmHandle             = g_UsbDeviceCompositeConfigList.config[2].classHandle;

		USB_DeviceCdcVcomInit       (&g_appHandle.UsbCompositeDev_handle);
		USB_DeviceAudioCompositeInit(&g_appHandle.UsbCompositeDev_handle);
    }

    USB_DeviceIsrEnable();

    /*Add one delay here to make the DP pull down long enough to allow host to detect the previous disconnection.*/
    SDK_DelayAtLeastUs(5000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
    USB_DeviceRun(g_appHandle.UsbCompositeDev_handle.deviceHandle);
}


