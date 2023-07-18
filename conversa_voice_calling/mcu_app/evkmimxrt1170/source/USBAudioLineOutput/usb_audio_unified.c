/*
 * Copyright (c) 2015 - 2016, Freescale Semiconductor, Inc.
 * Copyright 2016,2019,2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "appGlobal.h"
#include "usb_audio_unified.h"
#include "fsl_device_registers.h"

#include "board.h"
#include <stdio.h>
#include <stdlib.h>
#include "arm_math.h"

// memory section replacement
#include <cr_section_macros.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define USB_AUDIO_ENTER_CRITICAL() 			\
				OSA_SR_ALLOC();            	\
				OSA_ENTER_CRITICAL()
#define USB_AUDIO_EXIT_CRITICAL() OSA_EXIT_CRITICAL()


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
usb_status_t USB_DeviceAudioCallback(class_handle_t handle, uint32_t event, void *param);


#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
//extern void SCTIMER_CaptureInit(void);
#endif


/*******************************************************************************
 * Variables
 ******************************************************************************/
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t audioPlayDataBuff[AUDIO_SPEAKER_DATA_WHOLE_BUFFER_LENGTH * FS_ISO_OUT_ENDP_PACKET_SIZE_MAX];
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t audioPlayPacket[FS_ISO_OUT_ENDP_PACKET_SIZE];
#else
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t audioPlayPacket[(FS_ISO_OUT_ENDP_PACKET_SIZE_MAX + APP_USB_RX_CHANNEL_MAX * APP_USB_RX_BYTES_MAX)];
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t audioFeedBackBuffer[HS_ISO_FEEDBACK_ENDP_PACKET_SIZE];
volatile uint8_t feedbackValueUpdating;
#endif
usb_status_t status;
uint8_t UsbAudioUpStreamingIsStarted;
uint8_t UsbAudioDnStreamingIsStarted;
//extern usb_device_endpoint_struct_t g_UsbDeviceAudioRecorderEndpoints[USB_AUDIO_RECORDER_STREAM_ENDPOINT_COUNT];
//extern usb_device_endpoint_struct_t g_UsbDeviceAudioSpeakerEndpoints[USB_AUDIO_SPEAKER_STREAM_ENDPOINT_COUNT];
usb_device_composite_struct_t *g_deviceAudioComposite;
volatile bool g_CodecMuteUnmute = false;
extern int16_t AllZeroBuf_48PointsSingleCh_16Bit[APP_USB_TX_CHANNEL_MAX*(APP_USB_TX_SAMPLING_RATE_KHZ_MAX+1)];
extern usb_device_class_struct_t g_UsbDeviceCdcVcomConfig;
extern usb_device_class_struct_t g_UsbDeviceHidMouseClass;
extern usb_device_class_struct_t g_UsbDeviceAudioClassRecorder;
extern usb_device_class_struct_t g_UsbDeviceAudioClassSpeaker;
extern usb_device_composite_struct_t *g_UsbDeviceComposite;
extern usb_device_composite_struct_t *g_deviceAudioComposite;
static usb_device_class_config_struct_t g_CompositeClassConfig[
																#ifdef UsbCompostieDev_VCOM_AUDIO
																	3
																#endif
															  ] =
{
	{
		USB_DeviceCdcVcomCallback,
		(class_handle_t) NULL,
		&g_UsbDeviceCdcVcomConfig,
	},
	#ifdef UsbCompostieDev_VCOM_AUDIO
		{
			USB_DeviceAudioCompositeCallback,
			(class_handle_t) NULL,
			&g_UsbDeviceAudioClassRecorder,
		},
		{
			USB_DeviceAudioCompositeCallback,
			(class_handle_t) NULL,
			&g_UsbDeviceAudioClassSpeaker,
		}
	#endif
};
/* USB device class configuration information */
 usb_device_class_config_list_struct_t g_UsbDeviceCompositeConfigList =
{
    g_CompositeClassConfig,
    USB_DeviceCallback,
	#ifdef UsbCompostieDev_VCOM_AUDIO
		3
	#endif
};

usb_phy_config_struct_t g_phyConfig =
	{
        BOARD_USB_PHY_D_CAL,
        BOARD_USB_PHY_TXCAL45DP,
        BOARD_USB_PHY_TXCAL45DM,
    };

/*******************************************************************************
 * Code
 ******************************************************************************/
// USB port1  IRQ
void USB_OTG1_IRQHandler(void)
{

#ifdef MIPS_MEASURE_GPIO
	GPIO_PinWrite(BOARD_USER_MIPS_GPIO_1, BOARD_USER_MIPS_GPIO_1_PIN, 1); // BOARD_USER_MIPS_GPIO_1_PURPOSE_TEXT pin to 1
#endif

	USB_DeviceEhciIsrFunction(g_appHandle.usbTxRx_handle.deviceHandle);

#ifdef MIPS_MEASURE_GPIO
	GPIO_PinWrite(BOARD_USER_MIPS_GPIO_1, BOARD_USER_MIPS_GPIO_1_PIN, 0); // BOARD_USER_MIPS_GPIO_1_PURPOSE_TEXT pin to 0
#endif
}
// USB port2  IRQ
void USB_OTG2_IRQHandler(void)
{
    USB_DeviceEhciIsrFunction(g_appHandle.usbTxRx_handle.deviceHandle);
}

#if ((defined(USB_DEVICE_CONFIG_AUDIO)) && (USB_DEVICE_CONFIG_AUDIO > 0U))

/*!
 * @brief Audio class specific request function.
 *
 * This function handles the Audio class specific requests.
 *
 * @param handle           The USB device handle.
 * @param event            The USB device event type.
 * @param param            The parameter of the device specific request.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
__RAMFUNC(RAM) // force to put the code instruction of this function in RAM (FLEXRAMRAM_ITC)
usb_status_t USB_DeviceCallback(usb_device_handle handle, uint32_t event, void *param)
{
    usb_status_t error = kStatus_USB_InvalidRequest;
    uint16_t *temp16   = (uint16_t *)param;
    uint8_t *temp8     = (uint8_t *)param;
    uint8_t count      = 0U;
    uint8_t localBuff[HS_ISO_IN_ENDP_PACKET_SIZE_MAX];
    uint32_t audio_sampling_rate_to_10_14 = 0;
    uint32_t audio_sampling_rate_to_16_16 = 0;
    audio_sampling_rate_to_10_14 = (g_appHandle.audioDefinition.audioRxPath_handle.configParam.sampleRate/1000)<<10;
    audio_sampling_rate_to_16_16 = (g_appHandle.audioDefinition.audioRxPath_handle.configParam.sampleRate/1000)<<13;

    switch (event)
    {
        case kUSB_DeviceEventBusReset:
        {
            for (count = 0U; count < USB_DEVICE_INTERFACE_COUNT; count++)
            {
            	g_appHandle.usbTxRx_handle.currentInterfaceAlternateSetting[count] = 0U;
            }
            /* reset audio speaker status to be the initialized status */
            USB_DeviceAudioSpeakerStatusReset();
			#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
			#else
				/* reset the the last feedback value */
            //g_appHandle.usbTxRx_handle.audioUnified.lastFeedbackValue             = audio_sampling_rate_to_10_14;
			#endif
            g_appHandle.usbTxRx_handle.attach               = 0U;
            g_appHandle.usbTxRx_handle.currentConfiguration = 0U;
            error                            = kStatus_USB_Success;
#if (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)) || (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
            /* Get USB speed to configure the device, including max packet size and interval of the endpoints. */
            if (kStatus_USB_Success == USB_DeviceClassGetSpeed(CONTROLLER_ID, &g_appHandle.usbTxRx_handle.speed))
            {
                USB_DeviceSetSpeed(handle, g_appHandle.usbTxRx_handle.speed);

            }
            if (USB_SPEED_HIGH == g_appHandle.usbTxRx_handle.speed)
            {
				#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
					g_composite.audioUnified.currentStreamOutMaxPacketSize = (HS_ISO_OUT_ENDP_PACKET_SIZE);
				#else
					g_appHandle.usbTxRx_handle.audioUnified.currentStreamOutMaxPacketSize =
						(g_appHandle.usbTxRx_handle.audioUnified.iso_out_endp_packet_size + g_appHandle.audioDefinition.audioRxPath_handle.configParam.channelNumber * g_appHandle.audioDefinition.audioRxPath_handle.configParam.bitPerSample/8);
					g_appHandle.usbTxRx_handle.audioUnified.currentFeedbackMaxPacketSize = HS_ISO_FEEDBACK_ENDP_PACKET_SIZE;
				#endif /* USB_DEVICE_AUDIO_USE_SYNC_MODE */

				#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
					/* high speed and audio 2.0, audio play interval is set by HS EP packet size */
					g_appHandle.usbTxRx_handle.audioUnified.audioPlayTransferSize = g_appHandle.usbTxRx_handle.audioUnified.iso_out_endp_packet_size;
					/* use short play buffer size, only use two elements */
					g_appHandle.usbTxRx_handle.audioUnified.audioPlayBufferSize = APP_USB_RX_CIRCULAR_BUFFER_SIZE_BYTE;
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
							g_appHandle.usbTxRx_handle.audioUnified.audioPlayTransferSize = g_appHandle.usbTxRx_handle.audioUnified.iso_out_endp_packet_size;//AUDIO_PLAY_BUFFER_SIZE_ONE_FRAME;
					/* use the whole play buffer size */
							g_appHandle.usbTxRx_handle.audioUnified.audioPlayBufferSize =  APP_USB_RX_CIRCULAR_BUFFER_SIZE_BYTE;
					#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
					#else
						AUDIO_UPDATE_FEEDBACK_DATA(audioFeedBackBuffer, audio_sampling_rate_to_10_14);
					#endif
				#endif /* USB_DEVICE_CONFIG_AUDIO_CLASS_2_0 */
                g_deviceAudioComposite->audioUnified.speed = USB_SPEED_HIGH;
            }
            else
            {
				#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
            		g_composite.audioUnified.currentStreamOutMaxPacketSize = (FS_ISO_OUT_ENDP_PACKET_SIZE);
				#else
            		g_appHandle.usbTxRx_handle.audioUnified.currentStreamOutMaxPacketSize =
            				(g_appHandle.usbTxRx_handle.audioUnified.iso_out_endp_packet_size + g_appHandle.audioDefinition.audioRxPath_handle.configParam.channelNumber * g_appHandle.audioDefinition.audioRxPath_handle.configParam.bitPerSample/8);
            		g_appHandle.usbTxRx_handle.audioUnified.currentFeedbackMaxPacketSize = FS_ISO_FEEDBACK_ENDP_PACKET_SIZE;
					AUDIO_UPDATE_FEEDBACK_DATA(audioFeedBackBuffer, audio_sampling_rate_to_10_14);
				#endif
                /* full speed, audio play interval is 1 ms using this play size */
					g_appHandle.usbTxRx_handle.audioUnified.audioPlayTransferSize = g_appHandle.usbTxRx_handle.audioUnified.iso_out_endp_packet_size;//AUDIO_PLAY_BUFFER_SIZE_ONE_FRAME;
                /* use the whole play buffer size */
					g_appHandle.usbTxRx_handle.audioUnified.audioPlayBufferSize = APP_USB_RX_CIRCULAR_BUFFER_SIZE_BYTE;

            }
#else
			#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
				g_composite.audioUnified.currentStreamOutMaxPacketSize = (FS_ISO_OUT_ENDP_PACKET_SIZE);
			#else
				g_composite.audioUnified.currentStreamOutMaxPacketSize =
					(FS_ISO_OUT_ENDP_PACKET_SIZE + AUDIO_OUT_FORMAT_CHANNELS * AUDIO_OUT_FORMAT_SIZE);
				g_composite.audioUnified.currentFeedbackMaxPacketSize = FS_ISO_FEEDBACK_ENDP_PACKET_SIZE;
				AUDIO_UPDATE_FEEDBACK_DATA(audioFeedBackBuffer, AUDIO_SAMPLING_RATE_TO_10_14);
				/* reset the the last feedback value */
				g_deviceAudioComposite->audioUnified.lastFeedbackValue = 0U;
			#endif
            /* full speed, audio play interval is 1 ms using this play size */
            g_composite.audioUnified.audioPlayTransferSize = AUDIO_PLAY_BUFFER_SIZE_ONE_FRAME;
            /* use the whole play buffer size */
            g_composite.audioUnified.audioPlayBufferSize = AUDIO_SPEAKER_DATA_WHOLE_BUFFER_LENGTH * AUDIO_PLAY_BUFFER_SIZE_ONE_FRAME;
#endif /* USB_DEVICE_CONFIG_EHCI, USB_DEVICE_CONFIG_LPCIP3511HS */

        }
        break;
        case kUSB_DeviceEventSetConfiguration:
            if (0U == (*temp8))
            {
            	g_appHandle.usbTxRx_handle.attach               = 0U;
            	g_appHandle.usbTxRx_handle.currentConfiguration = 0U;
                error                            = kStatus_USB_Success;
            }
            else if (USB_COMPOSITE_CONFIGURE_INDEX == (*temp8))
            {
            	g_appHandle.usbTxRx_handle.attach               = 1U;
            	g_appHandle.usbTxRx_handle.currentConfiguration = *temp8;
#ifdef UsbCompostieDev_VCOM_AUDIO
				USB_DeviceAudioCompositeSetConfigure(g_appHandle.usbTxRx_handle.audioUnified.audioSpeakerHandle, *temp8);
				USB_DeviceAudioCompositeSetConfigure(g_appHandle.usbTxRx_handle.audioUnified.audioRecorderHandle, *temp8);
#endif
				USB_DeviceCdcVcomSetConfigure(g_appHandle.usbTxRx_handle.cdcVcom.cdcAcmHandle, *temp8);
                error = kStatus_USB_Success;
            }
            else
            {
            }
            break;
        case kUSB_DeviceEventSetInterface:

            if (g_appHandle.usbTxRx_handle.attach)
            {
                uint8_t interface        = (uint8_t)((*temp16 & 0xFF00U) >> 0x08U);
                uint8_t alternateSetting = (uint8_t)(*temp16 & 0x00FFU);

                switch (interface)
                {
                    case USB_AUDIO_CONTROL_INTERFACE_INDEX:
                        if (alternateSetting < USB_AUDIO_CONTROL_INTERFACE_ALTERNATE_COUNT)
                        {
                            error = kStatus_USB_Success;
                        }
                        break;
#ifdef UsbCompostieDev_VCOM_AUDIO
                    case USB_AUDIO_RECORDER_STREAM_INTERFACE_INDEX:

                        if (alternateSetting < USB_AUDIO_RECORDER_STREAM_INTERFACE_ALTERNATE_COUNT)
                        {
                        	g_appHandle.usbTxRx_handle.currentInterfaceAlternateSetting[interface] = alternateSetting;
							error                                                   = kStatus_USB_Success;
							if (alternateSetting == 1)
							{
/*								USB_AudioReadCircularBufferData(	&g_USBCircularBuffer[g_appHandle.usbTxRx_handle.audioUnified.tdReadNumber],
																	&localBuff[0],
																	((USB_SPEED_HIGH == g_appHandle.usbTxRx_handle.speed) ? g_appHandle.usbTxRx_handle.audioUnified.iso_in_endp_packet_size : g_appHandle.usbTxRx_handle.audioUnified.iso_in_endp_packet_size) );
								toolsCircularBufferRead_I_I(&g_appHandle.usbTxRx_handle.cirBuffTx,
															&localBuff[0],
															(USB_SPEED_HIGH == g_appHandle.usbTxRx_handle.speed) ? g_appHandle.usbTxRx_handle.audioUnified.iso_in_endp_packet_size : g_appHandle.usbTxRx_handle.audioUnified.iso_in_endp_packet_size );
*/
								memset(&localBuff[0],0x0,g_appHandle.usbTxRx_handle.audioUnified.iso_in_endp_packet_size );
								error = USB_DeviceAudioSend( g_appHandle.usbTxRx_handle.audioUnified.audioRecorderHandle,
															USB_AUDIO_RECORDER_STREAM_ENDPOINT,
															 &localBuff[0],
															 (USB_SPEED_HIGH == g_appHandle.usbTxRx_handle.speed) ? g_appHandle.usbTxRx_handle.audioUnified.iso_in_endp_packet_size : g_appHandle.usbTxRx_handle.audioUnified.iso_in_endp_packet_size);

							}

                        }
                        break;
                    case USB_AUDIO_SPEAKER_STREAM_INTERFACE_INDEX:
                        if (alternateSetting < USB_AUDIO_SPEAKER_STREAM_INTERFACE_ALTERNATE_COUNT)
                        {
                            error = USB_DeviceAudioSpeakerSetInterface(g_appHandle.usbTxRx_handle.audioUnified.audioSpeakerHandle,
                                                                       interface, alternateSetting);
							#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
								if (USB_AUDIO_SPEAKER_STREAM_INTERFACE_ALTERNATE_1 == alternateSetting)
								{
									g_composite.audioUnified.stopDataLengthAudioAdjust = 0U;
								}
								else if (USB_AUDIO_SPEAKER_STREAM_INTERFACE_ALTERNATE_0 == alternateSetting)
								{
									g_composite.audioUnified.stopDataLengthAudioAdjust = 1U;
								}
							#else
								/* usb host stops the speaker, so there is no need for feedback */
								if ((1U == g_appHandle.usbTxRx_handle.audioUnified.startPlay) &&  (USB_AUDIO_SPEAKER_STREAM_INTERFACE_ALTERNATE_0 == alternateSetting))
								{
									g_appHandle.usbTxRx_handle.audioUnified.stopFeedbackUpdate = 1U;
								}

								/* usb host start the speaker, discard the feedback for AUDIO_SPEAKER_FEEDBACK_DISCARD_COUNT
								 * times */
								if (USB_AUDIO_SPEAKER_STREAM_INTERFACE_ALTERNATE_1 == alternateSetting)
								{
									g_appHandle.usbTxRx_handle.audioUnified.stopFeedbackUpdate              = 0U;
									g_deviceAudioComposite->audioUnified.feedbackDiscardFlag = 1U;
									//g_deviceAudioComposite->audioUnified.feedbackDiscardTimes = AUDIO_SPEAKER_FEEDBACK_DISCARD_COUNT;
								}
							#endif
                        }
                        break;

#endif
                    default:
                        break;
                }

                if (kStatus_USB_Success == error)
                {
                	g_appHandle.usbTxRx_handle.currentInterfaceAlternateSetting[interface] = alternateSetting;
                }
            }
            break;
        case kUSB_DeviceEventGetConfiguration:
            if (param)
            {
                *temp8 = g_appHandle.usbTxRx_handle.currentConfiguration;
                error  = kStatus_USB_Success;
            }
            break;
        case kUSB_DeviceEventGetInterface:
            if (param)
            {
                uint8_t interface = (uint8_t)((*temp16 & 0xFF00U) >> 0x08U);
                if (interface < USB_DEVICE_INTERFACE_COUNT)
                {
                    *temp16 = (*temp16 & 0xFF00U) | g_appHandle.usbTxRx_handle.currentInterfaceAlternateSetting[interface];
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
__RAMFUNC(RAM) // force to put the code instruction of this function in RAM (FLEXRAMRAM_ITC)
usb_status_t USB_DeviceAudioRequest(class_handle_t handle, uint32_t event, void *param) {
    usb_device_control_request_struct_t *request = (usb_device_control_request_struct_t *)param;
    usb_status_t error                           = kStatus_USB_Success;
#if (!USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
    uint16_t volume;
#endif

    switch (event)
    {
        case USB_DEVICE_AUDIO_FU_GET_CUR_MUTE_CONTROL:
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            request->buffer = (uint8_t *)&g_deviceAudioComposite->audioUnified.curMute20;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.curMute20);
#else
            request->buffer = &g_deviceAudioComposite->audioUnified.curMute;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.curMute);
#endif
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_VOLUME_CONTROL:
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            request->buffer = (uint8_t *)&g_deviceAudioComposite->audioUnified.curVolume20;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.curVolume20);
#else
            request->buffer = g_deviceAudioComposite->audioUnified.curVolume;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.curVolume);
#endif
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_BASS_CONTROL:
            request->buffer = &g_deviceAudioComposite->audioUnified.curBass;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.curBass);
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_MID_CONTROL:
            request->buffer = &g_deviceAudioComposite->audioUnified.curMid;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.curMid);
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_TREBLE_CONTROL:
            request->buffer = &g_deviceAudioComposite->audioUnified.curTreble;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.curTreble);
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_AUTOMATIC_GAIN_CONTROL:
            request->buffer = &g_deviceAudioComposite->audioUnified.curAutomaticGain;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.curAutomaticGain);
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_DELAY_CONTROL:
            request->buffer = g_deviceAudioComposite->audioUnified.curDelay;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.curDelay);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_VOLUME_CONTROL:
            request->buffer = g_deviceAudioComposite->audioUnified.minVolume;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.minVolume);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_BASS_CONTROL:
            request->buffer = &g_deviceAudioComposite->audioUnified.minBass;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.minBass);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_MID_CONTROL:
            request->buffer = &g_deviceAudioComposite->audioUnified.minMid;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.minMid);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_TREBLE_CONTROL:
            request->buffer = &g_deviceAudioComposite->audioUnified.minTreble;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.minTreble);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_DELAY_CONTROL:
            request->buffer = g_deviceAudioComposite->audioUnified.minDelay;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.minDelay);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_VOLUME_CONTROL:
            request->buffer = g_deviceAudioComposite->audioUnified.maxVolume;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.maxVolume);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_BASS_CONTROL:
            request->buffer = &g_deviceAudioComposite->audioUnified.maxBass;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.maxBass);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_MID_CONTROL:
            request->buffer = &g_deviceAudioComposite->audioUnified.maxMid;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.maxMid);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_TREBLE_CONTROL:
            request->buffer = &g_deviceAudioComposite->audioUnified.maxTreble;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.maxTreble);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_DELAY_CONTROL:
            request->buffer = g_deviceAudioComposite->audioUnified.maxDelay;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.maxDelay);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_VOLUME_CONTROL:
            request->buffer = g_deviceAudioComposite->audioUnified.resVolume;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.resVolume);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_BASS_CONTROL:
            request->buffer = &g_deviceAudioComposite->audioUnified.resBass;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.resBass);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_MID_CONTROL:
            request->buffer = &g_deviceAudioComposite->audioUnified.resMid;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.resMid);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_TREBLE_CONTROL:
            request->buffer = &g_deviceAudioComposite->audioUnified.resTreble;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.resTreble);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_DELAY_CONTROL:
            request->buffer = g_deviceAudioComposite->audioUnified.resDelay;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.resDelay);
            break;
#if USB_DEVICE_CONFIG_AUDIO_CLASS_2_0
        case USB_DEVICE_AUDIO_CS_GET_CUR_SAMPLING_FREQ_CONTROL:
            request->buffer = (uint8_t *)&g_deviceAudioComposite->audioUnified.curSampleFrequency;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.curSampleFrequency);
            break;
        case USB_DEVICE_AUDIO_CS_SET_CUR_SAMPLING_FREQ_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = (uint8_t *)&g_deviceAudioComposite->audioUnified.curSampleFrequency;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.curSampleFrequency);
            }
            break;
        case USB_DEVICE_AUDIO_CS_GET_CUR_CLOCK_VALID_CONTROL:
            request->buffer = &g_deviceAudioComposite->audioUnified.curClockValid;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.curClockValid);
            break;
        case USB_DEVICE_AUDIO_CS_SET_CUR_CLOCK_VALID_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_deviceAudioComposite->audioUnified.curClockValid;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.curClockValid);
            }
            break;
        case USB_DEVICE_AUDIO_CS_GET_RANGE_SAMPLING_FREQ_CONTROL:
            request->buffer = (uint8_t *)&g_deviceAudioComposite->audioUnified.freqControlRange;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.freqControlRange);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RANGE_VOLUME_CONTROL:
            request->buffer = (uint8_t *)&g_deviceAudioComposite->audioUnified.volumeControlRange;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.volumeControlRange);
            break;
#else
        case USB_DEVICE_AUDIO_EP_GET_CUR_SAMPLING_FREQ_CONTROL:
            request->buffer = g_deviceAudioComposite->audioUnified.curSamplingFrequency;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.curSamplingFrequency);
            break;
        case USB_DEVICE_AUDIO_EP_GET_MIN_SAMPLING_FREQ_CONTROL:
            request->buffer = g_deviceAudioComposite->audioUnified.minSamplingFrequency;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.minSamplingFrequency);
            break;
        case USB_DEVICE_AUDIO_EP_GET_MAX_SAMPLING_FREQ_CONTROL:
            request->buffer = g_deviceAudioComposite->audioUnified.maxSamplingFrequency;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.maxSamplingFrequency);
            break;
        case USB_DEVICE_AUDIO_EP_GET_RES_SAMPLING_FREQ_CONTROL:
            request->buffer = g_deviceAudioComposite->audioUnified.resSamplingFrequency;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.resSamplingFrequency);
            break;
        case USB_DEVICE_AUDIO_EP_SET_CUR_SAMPLING_FREQ_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_deviceAudioComposite->audioUnified.curSamplingFrequency;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.curSamplingFrequency);
            }
            break;
        case USB_DEVICE_AUDIO_EP_SET_RES_SAMPLING_FREQ_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_deviceAudioComposite->audioUnified.resSamplingFrequency;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.resSamplingFrequency);
            }
            break;
        case USB_DEVICE_AUDIO_EP_SET_MAX_SAMPLING_FREQ_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_deviceAudioComposite->audioUnified.maxSamplingFrequency;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.maxSamplingFrequency);
            }
            break;
        case USB_DEVICE_AUDIO_EP_SET_MIN_SAMPLING_FREQ_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_deviceAudioComposite->audioUnified.minSamplingFrequency;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.minSamplingFrequency);
            }
            break;
#endif
        case USB_DEVICE_AUDIO_FU_SET_CUR_VOLUME_CONTROL:
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            if (request->isSetup == 1U)
            {
                request->buffer = g_deviceAudioComposite->audioUnified.curVolume20;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.curVolume20);
            }
            else
            {
                g_deviceAudioComposite->audioUnified.codecTask |= VOLUME_CHANGE_TASK;
            }
#else
            if (request->isSetup == 1U)
            {
                request->buffer = g_deviceAudioComposite->audioUnified.curVolume;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.curVolume);
            }
            else
            {
                volume = (uint16_t)((uint16_t)g_deviceAudioComposite->audioUnified.curVolume[1] << 8U);
                volume |= (uint8_t)(g_deviceAudioComposite->audioUnified.curVolume[0]);
                g_deviceAudioComposite->audioUnified.codecTask |= VOLUME_CHANGE_TASK;
            }
#endif
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_MUTE_CONTROL:
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            if (request->isSetup == 1U)
            {
                request->buffer = &g_deviceAudioComposite->audioUnified.curMute20;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.curMute20);
            }
            else
            {
                if (g_deviceAudioComposite->audioUnified.curMute20)
                {
                    g_deviceAudioComposite->audioUnified.codecTask |= MUTE_CODEC_TASK;
                }
                else
                {
                    g_deviceAudioComposite->audioUnified.codecTask |= UNMUTE_CODEC_TASK;
                }
            }
#else
            if (request->isSetup == 1U)
            {
                request->buffer = &g_deviceAudioComposite->audioUnified.curMute;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.curMute);
            }
            else
            {
                if (g_deviceAudioComposite->audioUnified.curMute)
                {
                    g_deviceAudioComposite->audioUnified.codecTask |= MUTE_CODEC_TASK;
                }
                else
                {
                    g_deviceAudioComposite->audioUnified.codecTask |= UNMUTE_CODEC_TASK;
                }
            }
#endif
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_BASS_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_deviceAudioComposite->audioUnified.curBass;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.curBass);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_MID_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_deviceAudioComposite->audioUnified.curMid;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.curMid);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_TREBLE_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_deviceAudioComposite->audioUnified.curTreble;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.curTreble);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_AUTOMATIC_GAIN_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_deviceAudioComposite->audioUnified.curAutomaticGain;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.curAutomaticGain);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_DELAY_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_deviceAudioComposite->audioUnified.curDelay;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.curDelay);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_VOLUME_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_deviceAudioComposite->audioUnified.minVolume;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.minVolume);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_BASS_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_deviceAudioComposite->audioUnified.minBass;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.minBass);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_MID_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_deviceAudioComposite->audioUnified.minMid;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.minMid);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_TREBLE_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_deviceAudioComposite->audioUnified.minTreble;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.minTreble);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_DELAY_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_deviceAudioComposite->audioUnified.minDelay;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.minDelay);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_VOLUME_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_deviceAudioComposite->audioUnified.maxVolume;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.maxVolume);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_BASS_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_deviceAudioComposite->audioUnified.maxBass;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.maxBass);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_MID_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_deviceAudioComposite->audioUnified.maxMid;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.maxMid);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_TREBLE_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_deviceAudioComposite->audioUnified.maxTreble;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.maxTreble);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_DELAY_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_deviceAudioComposite->audioUnified.maxDelay;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.maxDelay);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_VOLUME_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_deviceAudioComposite->audioUnified.resVolume;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.resVolume);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_BASS_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_deviceAudioComposite->audioUnified.resBass;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.resBass);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_MID_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_deviceAudioComposite->audioUnified.resMid;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.resMid);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_TREBLE_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_deviceAudioComposite->audioUnified.resTreble;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.resTreble);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_DELAY_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_deviceAudioComposite->audioUnified.resDelay;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.resDelay);
            }
            break;
        default:
            error = kStatus_USB_InvalidRequest;
            break;
    }
    return error;
}
__RAMFUNC(RAM) // force to put the code instruction of this function in RAM (FLEXRAMRAM_ITC)
PL_UINT32 USB_UplinkFeedbackPacketLengthAdjust()
{
	PL_UINT32 packetSizeInBytes;
	PL_UINT32 usedSpaceInBytes;
	usedSpaceInBytes                    = toolsCircularBufferReadableData(&g_appHandle.usbTxRx_handle.cirBuffTx);
	packetSizeInBytes                   = g_appHandle.usbTxRx_handle.audioUnified.iso_in_endp_packet_size;

	if (usedSpaceInBytes >= g_appHandle.usbTxRx_handle.cirBuffTx.highLimitInBytes)
	{	//need to send more = + 1 sample per channel
		packetSizeInBytes = packetSizeInBytes +(  ( g_appHandle.usbTxRx_handle.USBTxchannelNumber )
												* ( g_appHandle.audioDefinition.audioTxPath_handle.configParam.bitPerSample / 8 ));
	}
	else if (usedSpaceInBytes <= g_appHandle.usbTxRx_handle.cirBuffTx.lowLimitInBytes)
	{	//need to send less = -1 sample per channel
		packetSizeInBytes = packetSizeInBytes -(  ( g_appHandle.usbTxRx_handle.USBTxchannelNumber )
												* ( g_appHandle.audioDefinition.audioTxPath_handle.configParam.bitPerSample / 8 ));
	}
	else
	{
		// no need to increment or decrement the packet size
	}
	return packetSizeInBytes;
}
__RAMFUNC(RAM) // force to put the code instruction of this function in RAM (FLEXRAMRAM_ITC)
usb_status_t USB_DownlinkFeedbackUpdate()
{


	usb_status_t retStatus;
    PL_UINT32    usedSpaceInBytes;
    PL_UINT32	 feedbackValue;
    PL_INT32 	 packetSizeInBytes;
    PL_INT32 	 packetSizeInSamplesPerChannel;

    //get packetSize
/*	packetSizeInBytes             =   ( g_appHandle.audioDefinition.audioRxPath_handle.configParam.sampleRate / 1000 )
									* ( g_appHandle.usbTxRx_handle.USBRxchannelNumber )
									*  3 /*( g_appHandle.audioDefinition.audioRxPath_handle.configParam.bitPerSample / 8 )*/;

    packetSizeInSamplesPerChannel = g_appHandle.audioDefinition.audioRxPath_handle.configParam.sampleRate / 1000;

    // reset and update counters
    g_appHandle.usbTxRx_handle.audioUnified.speakerIntervalCount = 1;
    g_appHandle.usbTxRx_handle.audioUnified.timesFeedbackCalculate++;
    if (g_appHandle.usbTxRx_handle.audioUnified.timesFeedbackCalculate <= 2)
    {
    	feedbackValue = packetSizeInSamplesPerChannel <<10;  // the feedback value must be converted to the 10.10 format
		if (USB_SPEED_HIGH == g_appHandle.usbTxRx_handle.speed)
		{
	        AUDIO_UPDATE_FEEDBACK_DATA(audioFeedBackBuffer, feedbackValue);
		}
		if (USB_SPEED_FULL == g_appHandle.usbTxRx_handle.speed)
		{
	        AUDIO_UPDATE_FEEDBACK_DATA(audioFeedBackBuffer, feedbackValue);
		}
		usedSpaceInBytes = (g_appHandle.usbTxRx_handle.cirBuffRx.cbSize>>1); // initialize the usedSpaceinBytes to the middleSize
    }
    else /* (g_deviceAudioComposite->audioUnified.timesFeedbackCalculate > 2)*/
    {

    	usedSpaceInBytes                    = toolsCircularBufferReadableData(&g_appHandle.usbTxRx_handle.cirBuffRx);
    	if (usedSpaceInBytes < g_appHandle.usbTxRx_handle.cirBuffRx.lowLimitInBytes) //=>  Host need to slow down
    	{

			feedbackValue = (packetSizeInSamplesPerChannel <<10) +64; //in 10.10 format
			//g_feedbackValue = (feedbackValue);
			//g_usedSpaceInBytes = usedSpaceInBytes;
			//g_feedbackFlag = 1;

    	}
    	else if (usedSpaceInBytes > g_appHandle.usbTxRx_handle.cirBuffRx.highLimitInBytes) // => host need to speed up
    	{
			feedbackValue = (packetSizeInSamplesPerChannel <<10) -64 ; // in 10.10 format
			//g_feedbackValue = (feedbackValue);
			//g_usedSpaceInBytes = usedSpaceInBytes;
			//g_feedbackFlag = 1;
    	}
    	else /* lowLimitInBytes < usedSpaceInBytes < highLimitInBytes */
    	{
    		feedbackValue = packetSizeInSamplesPerChannel <<10;  				 // the feedback value must be converted to the 10.10 format
    	}
    	AUDIO_UPDATE_FEEDBACK_DATA(audioFeedBackBuffer, feedbackValue); 		 // update the feedback buffer
#ifdef LetComFeedbackUsbUpDnStreamBufLevel
		VComReportValue_FeedbackEpValue        =feedbackValue;
#endif
    }

}

/*!
 * @brief device Audio callback function.
 *
 * This function handle the Audio class specified event.
 * @param handle          The USB class  handle.
 * @param event           The USB device event type.
 * @param param           The parameter of the class specific event.
 * @return kStatus_USB_Success or error.
 */
//__RAMFUNC(RAM) // force to put the code instruction of this function in RAM (FLEXRAMRAM_ITC)
uint8_t localBuff[HS_ISO_IN_ENDP_PACKET_SIZE_MAX];
__RAMFUNC(RAM) // force to put the code instruction of this function in RAM (FLEXRAMRAM_ITC)
usb_status_t USB_DeviceAudioCompositeCallback(class_handle_t handle, uint32_t event, void *param) {    					/* USB CALLBACK USE FOR COMPOSITE */
	usb_status_t error = kStatus_USB_Error;
	usb_device_endpoint_callback_message_struct_t *ep_cb_param;

	ep_cb_param = (usb_device_endpoint_callback_message_struct_t*) param;

	uint32_t USBpacketSize_byte = 0;
	int i;
	switch (event)
	{
	case kUSB_DeviceAudioEventStreamSendResponse: // USB Tx send data to PC
		/* endpoint callback length is USB_CANCELLED_TRANSFER_LENGTH (0xFFFFFFFFU) when transfer is canceled */
		            if ((g_deviceAudioComposite->audioUnified.attach) &&
		                (ep_cb_param->length != (USB_CANCELLED_TRANSFER_LENGTH)))
		            {
		                if (ep_cb_param->length == ((USB_SPEED_HIGH == g_appHandle.usbTxRx_handle.audioUnified.speed) ? HS_ISO_FEEDBACK_ENDP_PACKET_SIZE :
		                                                                                    							FS_ISO_FEEDBACK_ENDP_PACKET_SIZE))
		                {
		                	error = USB_DownlinkFeedbackUpdate();
							// Send Feedback for RX part
							error = USB_DeviceAudioSend(g_appHandle.usbTxRx_handle.audioUnified.audioSpeakerHandle,
														USB_AUDIO_SPEAKER_FEEDBACK_ENDPOINT,
														audioFeedBackBuffer,
														(USB_SPEED_HIGH == g_appHandle.usbTxRx_handle.audioUnified.speed) ? HS_ISO_FEEDBACK_ENDP_PACKET_SIZE :
																															FS_ISO_FEEDBACK_ENDP_PACKET_SIZE);
		                }
		                else
		                {
		                	if (g_appHandle.usbTxRx_handle.audioUnified.startRecording ==0)
							{
								g_appHandle.usbTxRx_handle.audioUnified.startRecording =1;
							}

		                    if (g_appHandle.usbTxRx_handle.cirBuffTx.flagMidFull ==1)
		                    {

								USB_AUDIO_ENTER_CRITICAL();
								USBpacketSize_byte =  USB_UplinkFeedbackPacketLengthAdjust();
								USB_AUDIO_EXIT_CRITICAL();
		                        toolsCircularBufferRead_I_I(&g_appHandle.usbTxRx_handle.cirBuffTx,
															&localBuff[0],
															USBpacketSize_byte);

		                        error = USB_DeviceAudioSend( g_deviceAudioComposite->audioUnified.audioRecorderHandle,						// USB handle
															 USB_AUDIO_RECORDER_STREAM_ENDPOINT,
															 &localBuff[0],																	// source,  local buffer start address
															 USBpacketSize_byte);															// size in byte


		                    }
		                    else
		                    {
		                    	memset(&localBuff[0],0x0,g_deviceAudioComposite->audioUnified.iso_in_endp_packet_size);
								error = USB_DeviceAudioSend( g_deviceAudioComposite->audioUnified.audioRecorderHandle,						// USB handle
															 USB_AUDIO_RECORDER_STREAM_ENDPOINT,
															 &localBuff[0],														// source,  local buffer start address
															 g_deviceAudioComposite->audioUnified.iso_in_endp_packet_size);		// size in byte
		                    }

		                }
		            }





		break;
	case kUSB_DeviceAudioEventStreamRecvResponse: // USB Rx receive data from PC
		if ((g_appHandle.usbTxRx_handle.audioUnified.attach) && (ep_cb_param->length != (USB_UNINITIALIZED_VAL_32)))
		{

			g_appHandle.usbTxRx_handle.audioUnified.usbRecvCount += ep_cb_param->length;
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
            error = USB_DeviceAudioRecv	( handle,
            							  USB_AUDIO_SPEAKER_STREAM_ENDPOINT,
										  &audioPlayPacket[0],
                                          (FS_ISO_OUT_ENDP_PACKET_SIZE));
#else
			// Feed audioPlayPacket with USb RX data

			error = USB_DeviceAudioRecv( handle,
										 USB_AUDIO_SPEAKER_STREAM_ENDPOINT,
										 &audioPlayPacket[0],
										 ((ep_cb_param->length) + g_appHandle.audioDefinition.audioRxPath_handle.configParam.channelNumber * (g_appHandle.usbTxRx_handle.USBRxBitDepth/8 )));

			// if Circular buffer midfull, raise up the midFull flag
			if (g_appHandle.usbTxRx_handle.cirBuffRx.flagMidFull ==0)
			{
			    if (toolsCircularBufferReadableData(&g_appHandle.usbTxRx_handle.cirBuffRx) > (g_appHandle.usbTxRx_handle.cirBuffRx.cbSize/2))
				{				
					g_appHandle.usbTxRx_handle.cirBuffRx.flagMidFull = 1;			
				}			
			}
			// Write Interleaved data from audioPlayPacket to Circular buffer RX
			toolsCircularBufferWrite_I_I(&g_appHandle.usbTxRx_handle.cirBuffRx,
										 audioPlayPacket,
										 ep_cb_param->length);

#endif

		}
		break;

	default:
		if (param && (event > 0xFF)) {
			// if another event like volume control
			error = USB_DeviceAudioRequest(handle, event, param);
		}
		break;
	}
	return error;
}

/* The USB_DeviceAudioSpeakerStatusReset() function resets the audio speaker status to the initialized status */
void USB_DeviceAudioSpeakerStatusReset(void) {
	g_deviceAudioComposite->audioUnified.startPlay = 0;
	g_deviceAudioComposite->audioUnified.startRecording = 0;
	g_deviceAudioComposite->audioUnified.audioSendCount = 0;
	g_deviceAudioComposite->audioUnified.usbRecvCount = 0;
	g_deviceAudioComposite->audioUnified.lastAudioSendCount = 0;

	g_deviceAudioComposite->audioUnified.speakerIntervalCount = 0;
	g_deviceAudioComposite->audioUnified.UsbDnStrmBuf_OccupiedSpaceInByte = 0;
	g_deviceAudioComposite->audioUnified.timesFeedbackCalculate = 0;
	g_deviceAudioComposite->audioUnified.speakerDetachOrNoInput = 0;
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
    g_deviceAudioComposite->audioUnified.audioPllTicksPrev   = 0;
    g_deviceAudioComposite->audioUnified.audioPllTicksDiff   = 0;
    g_deviceAudioComposite->audioUnified.audioPllTicksEma    = AUDIO_PLL_USB1_SOF_INTERVAL_COUNT;
    g_deviceAudioComposite->audioUnified.audioPllTickEmaFrac = 0;
    g_deviceAudioComposite->audioUnified.audioPllStep        = AUDIO_PLL_FRACTIONAL_CHANGE_STEP;
#endif
}

/*!
 * @brief Audio set configuration function.
 *
 * This function sets configuration for msc class.
 *
 * @param handle The Audio class handle.
 * @param configure The Audio class configure index.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceAudioCompositeSetConfigure(class_handle_t handle, uint8_t configure) {
	if (USB_COMPOSITE_CONFIGURE_INDEX == configure) {
		g_deviceAudioComposite->audioUnified.attach = 1U;
	}
	return kStatus_USB_Success;
}

usb_status_t USB_DeviceAudioRecorderSetInterface(class_handle_t handle, uint8_t interface, uint8_t alternateSetting) {
    usb_status_t error = kStatus_USB_Success;



    if (alternateSetting == 1U)
    {
        UsbAudioUpStreamingIsStarted = 1;
        //usb_echo("audio upstreaming started\r\n");
        error = USB_DeviceAudioSend(g_deviceAudioComposite->audioUnified.audioRecorderHandle,
									USB_AUDIO_RECORDER_STREAM_ENDPOINT,
									(uint8_t *)AllZeroBuf_48PointsSingleCh_16Bit,
									g_appHandle.usbTxRx_handle.audioUnified.iso_in_endp_packet_size
									);
    }
    else
    {
        UsbAudioUpStreamingIsStarted = 0;
    }
    return error;
}

usb_status_t USB_DeviceAudioSpeakerSetInterface(class_handle_t handle, uint8_t interface, uint8_t alternateSetting) {
	usb_status_t error = kStatus_USB_Success;

	if (alternateSetting == 1U)
	{
		error = USB_DeviceAudioRecv(g_deviceAudioComposite->audioUnified.audioSpeakerHandle,
									USB_AUDIO_SPEAKER_STREAM_ENDPOINT,
									&audioPlayDataBuff[0],
									g_appHandle.usbTxRx_handle.audioUnified.iso_out_endp_packet_size//APP_USB_RX_CIRCULAR_BUFFER_SIZE_BYTE
									);
		if (error != kStatus_USB_Success)
		{
			return error;
		} else
		{
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
#else
			error = USB_DeviceAudioSend(g_appHandle.usbTxRx_handle.audioUnified.audioSpeakerHandle,
										USB_AUDIO_SPEAKER_FEEDBACK_ENDPOINT,
										audioFeedBackBuffer,
										(USB_SPEED_HIGH == g_appHandle.usbTxRx_handle.audioUnified.speed) ? HS_ISO_FEEDBACK_ENDP_PACKET_SIZE :
																											FS_ISO_FEEDBACK_ENDP_PACKET_SIZE);
#endif
			UsbAudioDnStreamingIsStarted=1;
		}
	}else
	{
		UsbAudioDnStreamingIsStarted=0;
	}
	return error;
}
/*!
 * @brief Audio init function.
 *
 * This function initializes the device with the composite device class information.
 *
 * @param device_composite          The pointer to the composite device structure.
 * @return kStatus_USB_Success .
 */
usb_status_t USB_DeviceAudioCompositeInit(usb_device_composite_struct_t *device_composite)
{
	uint32_t audio_sampling_rate_to_10_14 = 0;
	audio_sampling_rate_to_10_14 = (g_appHandle.audioDefinition.audioRxPath_handle.configParam.sampleRate/1000)<<10;

#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
    //SCTIMER_CaptureInit();
#else
	AUDIO_UPDATE_FEEDBACK_DATA(audioFeedBackBuffer, audio_sampling_rate_to_10_14);
#endif
	g_deviceAudioComposite = device_composite;
	g_deviceAudioComposite->audioUnified.copyProtect = 0x01U;
	g_deviceAudioComposite->audioUnified.curMute = 0x00U;
	g_deviceAudioComposite->audioUnified.curVolume[0] = 0x00U;
	g_deviceAudioComposite->audioUnified.curVolume[1] = 0x1fU;
	g_deviceAudioComposite->audioUnified.minVolume[0] = 0x00U;
	g_deviceAudioComposite->audioUnified.minVolume[1] = 0x00U;
	g_deviceAudioComposite->audioUnified.maxVolume[0] = 0x00U;
	g_deviceAudioComposite->audioUnified.maxVolume[1] = 0X43U;
	g_deviceAudioComposite->audioUnified.resVolume[0] = 0x01U;
	g_deviceAudioComposite->audioUnified.resVolume[1] = 0x00U;
	g_deviceAudioComposite->audioUnified.curBass = 0x00U;
	g_deviceAudioComposite->audioUnified.curBass = 0x00U;
	g_deviceAudioComposite->audioUnified.minBass = 0x80U;
	g_deviceAudioComposite->audioUnified.maxBass = 0x7FU;
	g_deviceAudioComposite->audioUnified.resBass = 0x01U;
	g_deviceAudioComposite->audioUnified.curMid = 0x00U;
	g_deviceAudioComposite->audioUnified.minMid = 0x80U;
	g_deviceAudioComposite->audioUnified.maxMid = 0x7FU;
	g_deviceAudioComposite->audioUnified.resMid = 0x01U;
	g_deviceAudioComposite->audioUnified.curTreble = 0x01U;
	g_deviceAudioComposite->audioUnified.minTreble = 0x80U;
	g_deviceAudioComposite->audioUnified.maxTreble = 0x7FU;
	g_deviceAudioComposite->audioUnified.resTreble = 0x01U;
	g_deviceAudioComposite->audioUnified.curAutomaticGain = 0x01U;
	g_deviceAudioComposite->audioUnified.curDelay[0] = 0x00U;
	g_deviceAudioComposite->audioUnified.curDelay[1] = 0x40U;
	g_deviceAudioComposite->audioUnified.minDelay[0] = 0x00U;
	g_deviceAudioComposite->audioUnified.minDelay[1] = 0x00U;
	g_deviceAudioComposite->audioUnified.maxDelay[0] = 0xFFU;
	g_deviceAudioComposite->audioUnified.maxDelay[1] = 0xFFU;
	g_deviceAudioComposite->audioUnified.resDelay[0] = 0x00U;
	g_deviceAudioComposite->audioUnified.resDelay[1] = 0x01U;
	g_deviceAudioComposite->audioUnified.curLoudness = 0x01U;
	g_deviceAudioComposite->audioUnified.curSamplingFrequency[0] = 0x00U;
	g_deviceAudioComposite->audioUnified.curSamplingFrequency[1] = 0x00U;
	g_deviceAudioComposite->audioUnified.curSamplingFrequency[2] = 0x01U;
	g_deviceAudioComposite->audioUnified.minSamplingFrequency[0] = 0x00U;
	g_deviceAudioComposite->audioUnified.minSamplingFrequency[1] = 0x00U;
	g_deviceAudioComposite->audioUnified.minSamplingFrequency[2] = 0x01U;
	g_deviceAudioComposite->audioUnified.maxSamplingFrequency[0] = 0x00U;
	g_deviceAudioComposite->audioUnified.maxSamplingFrequency[1] = 0x00U;
	g_deviceAudioComposite->audioUnified.maxSamplingFrequency[2] = 0x01U;
	g_deviceAudioComposite->audioUnified.resSamplingFrequency[0] = 0x00U;
	g_deviceAudioComposite->audioUnified.resSamplingFrequency[1] = 0x00U;
	g_deviceAudioComposite->audioUnified.resSamplingFrequency[2] = 0x01U;
#if USB_DEVICE_CONFIG_AUDIO_CLASS_2_0
    g_deviceAudioComposite->audioUnified.curMute20          = 0U;
    g_deviceAudioComposite->audioUnified.curClockValid      = 1U;
    g_deviceAudioComposite->audioUnified.curVolume20[0]     = 0x00U;
    g_deviceAudioComposite->audioUnified.curVolume20[1]     = 0x1FU;
    g_deviceAudioComposite->audioUnified.curSampleFrequency = g_appHandle.audioDefinition.audioTxPath_handle.configParam.sampleRate;

    g_deviceAudioComposite->audioUnified.freqControlRange.wNumSubRanges = 1 ;
    g_deviceAudioComposite->audioUnified.freqControlRange.wMIN          = g_appHandle.audioDefinition.audioTxPath_handle.configParam.sampleRate;	//now, Dn stream and up stream must be the same FS
    g_deviceAudioComposite->audioUnified.freqControlRange.wMAX          = g_appHandle.audioDefinition.audioTxPath_handle.configParam.sampleRate;	//now, Dn stream and up stream must be the same FS
    g_deviceAudioComposite->audioUnified.freqControlRange.wRES          = 0U;

    g_deviceAudioComposite->audioUnified.volumeControlRange.wNumSubRanges = 1U;
    g_deviceAudioComposite->audioUnified.volumeControlRange.wMIN          = 0x8001U;
    g_deviceAudioComposite->audioUnified.volumeControlRange.wMAX          = 0x7FFFU;
    g_deviceAudioComposite->audioUnified.volumeControlRange.wRES          = 1U;

#endif
	g_deviceAudioComposite->audioUnified.audioSendCount = 0;
	g_deviceAudioComposite->audioUnified.lastAudioSendCount = 0;
	g_deviceAudioComposite->audioUnified.lastUsedSpace = 0;
	g_deviceAudioComposite->audioUnified.usbRecvCount = 0;
	g_deviceAudioComposite->audioUnified.startPlay = 0;
	g_deviceAudioComposite->audioUnified.startRecording = 0;
	//g_deviceAudioComposite->audioUnified.startPlayHalfFull = 0;
	g_deviceAudioComposite->audioUnified.speakerIntervalCount = 0;
	g_deviceAudioComposite->audioUnified.UsbDnStrmBuf_OccupiedSpaceInByte = 0;
	g_deviceAudioComposite->audioUnified.timesFeedbackCalculate = 0;
	g_deviceAudioComposite->audioUnified.speakerDetachOrNoInput = 0;
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
    g_deviceAudioComposite->audioUnified.audioPllTicksPrev   = 0;
    g_deviceAudioComposite->audioUnified.audioPllTicksDiff   = 0;
    g_deviceAudioComposite->audioUnified.audioPllTicksEma    = AUDIO_PLL_USB1_SOF_INTERVAL_COUNT;
    g_deviceAudioComposite->audioUnified.audioPllTickEmaFrac = 0;
    g_deviceAudioComposite->audioUnified.audioPllStep        = AUDIO_PLL_FRACTIONAL_CHANGE_STEP;
#endif


    g_deviceAudioComposite->audioUnified.iso_in_endp_packet_size =   (( g_appHandle.audioDefinition.audioTxPath_handle.configParam.sampleRate / 1000 ))
    																* ( g_appHandle.usbTxRx_handle.USBTxchannelNumber )
																	* ( g_appHandle.audioDefinition.audioTxPath_handle.configParam.bitPerSample / 8 );

    g_deviceAudioComposite->audioUnified.iso_out_endp_packet_size =   ( g_appHandle.audioDefinition.audioRxPath_handle.configParam.sampleRate / 1000 )
       																* ( g_appHandle.usbTxRx_handle.USBRxchannelNumber )
   																	* ( g_appHandle.audioDefinition.audioRxPath_handle.configParam.bitPerSample / 8 )
																	+ (( g_appHandle.usbTxRx_handle.USBRxchannelNumber )
																	*  ( g_appHandle.usbTxRx_handle.USBRxchannelNumber )) ;   // byte per sample



    return kStatus_USB_Success;
}
extern void BOARD_SetCodecMuteUnmute(bool mute);
void USB_AudioCodecTask(void)
{
    if (g_deviceAudioComposite->audioUnified.codecTask & MUTE_CODEC_TASK)
    {
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
        usb_echo("Set Cur Mute : %x\r\n", g_deviceAudioComposite->audioUnified.curMute20);
#else
        usb_echo("Set Cur Mute : %x\r\n", g_deviceAudioComposite->audioUnified.curMute);
#endif
        BOARD_SetCodecMuteUnmute(true);
        g_deviceAudioComposite->audioUnified.codecTask &= ~MUTE_CODEC_TASK;
        g_CodecMuteUnmute = true;
    }
    if (g_deviceAudioComposite->audioUnified.codecTask & UNMUTE_CODEC_TASK)
    {
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
        usb_echo("Set Cur Mute : %x\r\n", g_deviceAudioComposite->audioUnified.curMute20);
#else
        usb_echo("Set Cur Mute : %x\r\n", g_deviceAudioComposite->audioUnified.curMute);
#endif
        BOARD_SetCodecMuteUnmute(false);
        g_deviceAudioComposite->audioUnified.codecTask &= ~UNMUTE_CODEC_TASK;
        g_CodecMuteUnmute = true;
    }
    if (g_deviceAudioComposite->audioUnified.codecTask & VOLUME_CHANGE_TASK)
    {
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
        usb_echo("Set Cur Volume : %x\r\n", (uint16_t)(g_deviceAudioComposite->audioUnified.curVolume20[1] << 8U) |
                                                g_deviceAudioComposite->audioUnified.curVolume20[0]);
#else
        usb_echo("Set Cur Volume : %x\r\n", (uint16_t)(g_deviceAudioComposite->audioUnified.curVolume[1] << 8U) |
                                                g_deviceAudioComposite->audioUnified.curVolume[0]);
#endif
        g_deviceAudioComposite->audioUnified.codecTask &= ~VOLUME_CHANGE_TASK;
    }
}






#endif
