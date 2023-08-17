/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


//Standard headers
#include <init.h>
#include <stdint.h>

//Platform headers
#include "board.h"

//fsl driver headers
#include "fsl_ctimer.h"

//Application headers
#include "appGlobal.h"
#include "usb_init.h"



/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* audio 2.0 and high speed, use low latency, but IP3511HS controller do not have micro frame count */
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
#if (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
volatile static uint8_t s_microFrameCountIp3511HS = 0;
#endif
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
usb_status_t USB_DeviceAudioCallback(class_handle_t handle, PL_UINT32 event, void *param);
usb_status_t USB_DeviceCallback(usb_device_handle handle, PL_UINT32 event, void *param);

extern void BOARD_SetCodecMuteUnmute(bool);
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
extern void CTIMER_CaptureInit(void);
#if defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
extern void audio_fro_trim_up(void);
extern void audio_fro_trim_down(void);
#endif
extern void USB_AudioPllChange(void);
#endif
/*******************************************************************************
 * Variables
 ******************************************************************************/
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)

#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t audioPlayPacket[APP_USB_RX_SAMPLING_RATE_KHZ_MAX * APP_USB_RX_CHANNEL_MAX *APP_USB_RX_BYTES_MAX*2];									//buffer size to be the max (of the original macro defined combinations)
#else
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t audioPlayPacket[(FS_ISO_OUT_ENDP_PACKET_SIZE + AUDIO_OUT_FORMAT_CHANNELS * AUDIO_OUT_FORMAT_SIZE)];
#endif

#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t audioRecPacket[APP_USB_TX_SAMPLING_RATE_KHZ_MAX *APP_USB_TX_CHANNEL_MAX * APP_USB_TX_BYTES_MAX *2 ];									//buffer size to be the max (of the original macro defined combinations)
#else
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t audioRecPacket[(FS_ISO_IN_ENDP_PACKET_SIZE + AUDIO_IN_FORMAT_CHANNELS * AUDIO_IN_FORMAT_SIZE)];
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t usbAudioFeedBackBuffer[4];
USB_RAM_ADDRESS_ALIGNMENT(4) uint8_t audioFeedBackBuffer[4];
volatile uint8_t feedbackValueUpdating;
#endif


volatile bool g_CodecSpeakerMuteUnmute    = false;
volatile bool g_CodecMicrophoneMuteUnmute = false;

/*******************************************************************************
 * Code
 ******************************************************************************/

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
usb_status_t USB_DeviceAudioRequest(class_handle_t handle, PL_UINT32 event, void *param)
{
    usb_device_control_request_struct_t *request = (usb_device_control_request_struct_t *)param;
    usb_status_t error                           = kStatus_USB_Success;
    uint8_t entity_id                            = (uint8_t)(request->setup->wIndex >> 0x08);
#if (!USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
    uint16_t volume;
#endif

    switch (event)
    {
        case USB_DEVICE_AUDIO_FU_GET_CUR_MUTE_CONTROL:
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            if (USB_AUDIO_SPEAKER_CONTROL_FEATURE_UNIT_ID == entity_id)
            {
                request->buffer = (uint8_t *)&g_UsbCompositeDevPtr->audioUnified.curSpeakerMute20;
                request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.curSpeakerMute20);
            }
            else if (USB_AUDIO_RECORDER_CONTROL_FEATURE_UNIT_ID == entity_id)
            {
                request->buffer = (uint8_t *)&g_UsbCompositeDevPtr->audioUnified.curMicrophoneMute20;
                request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.curMicrophoneMute20);
            }
            else
            {
                /* no action */
            }
#else
            if (USB_AUDIO_SPEAKER_CONTROL_FEATURE_UNIT_ID == entity_id)
            {
                request->buffer = (uint8_t *)&g_UsbCompositeDevPtr->audioUnified.curSpeakerMute;
                request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.curSpeakerMute);
            }
            else if (USB_AUDIO_RECORDER_CONTROL_FEATURE_UNIT_ID == entity_id)
            {
                request->buffer = (uint8_t *)&g_UsbCompositeDevPtr->audioUnified.curMicrophoneMute;
                request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.curMicrophoneMute);
            }
            else
            {
                /* no action */
            }
#endif
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_VOLUME_CONTROL:
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            if (USB_AUDIO_SPEAKER_CONTROL_FEATURE_UNIT_ID == entity_id)
            {
                request->buffer = (uint8_t *)&g_UsbCompositeDevPtr->audioUnified.curSpeakerVolume20;
                request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.curSpeakerVolume20);
            }
            else if (USB_AUDIO_RECORDER_CONTROL_FEATURE_UNIT_ID == entity_id)
            {
                request->buffer = (uint8_t *)&g_UsbCompositeDevPtr->audioUnified.curMicrophoneVolume20;
                request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.curMicrophoneVolume20);
            }
            else
            {
                /* no action */
            }
#else
            if (USB_AUDIO_SPEAKER_CONTROL_FEATURE_UNIT_ID == entity_id)
            {
                request->buffer = g_UsbCompositeDevPtr->audioUnified.curSpeakerVolume;
                request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.curSpeakerVolume);
            }
            else if (USB_AUDIO_RECORDER_CONTROL_FEATURE_UNIT_ID == entity_id)
            {
                request->buffer = g_UsbCompositeDevPtr->audioUnified.curMicrophoneVolume;
                request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.curMicrophoneVolume);
            }
            else
            {
                /* no action */
            }
#endif
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_BASS_CONTROL:
            request->buffer = &g_UsbCompositeDevPtr->audioUnified.curBass;
            request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.curBass);
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_MID_CONTROL:
            request->buffer = &g_UsbCompositeDevPtr->audioUnified.curMid;
            request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.curMid);
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_TREBLE_CONTROL:
            request->buffer = &g_UsbCompositeDevPtr->audioUnified.curTreble;
            request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.curTreble);
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_AUTOMATIC_GAIN_CONTROL:
            request->buffer = &g_UsbCompositeDevPtr->audioUnified.curAutomaticGain;
            request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.curAutomaticGain);
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_DELAY_CONTROL:
            request->buffer = g_UsbCompositeDevPtr->audioUnified.curDelay;
            request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.curDelay);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_VOLUME_CONTROL:
            request->buffer = g_UsbCompositeDevPtr->audioUnified.minVolume;
            request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.minVolume);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_BASS_CONTROL:
            request->buffer = &g_UsbCompositeDevPtr->audioUnified.minBass;
            request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.minBass);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_MID_CONTROL:
            request->buffer = &g_UsbCompositeDevPtr->audioUnified.minMid;
            request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.minMid);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_TREBLE_CONTROL:
            request->buffer = &g_UsbCompositeDevPtr->audioUnified.minTreble;
            request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.minTreble);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_DELAY_CONTROL:
            request->buffer = g_UsbCompositeDevPtr->audioUnified.minDelay;
            request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.minDelay);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_VOLUME_CONTROL:
            request->buffer = g_UsbCompositeDevPtr->audioUnified.maxVolume;
            request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.maxVolume);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_BASS_CONTROL:
            request->buffer = &g_UsbCompositeDevPtr->audioUnified.maxBass;
            request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.maxBass);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_MID_CONTROL:
            request->buffer = &g_UsbCompositeDevPtr->audioUnified.maxMid;
            request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.maxMid);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_TREBLE_CONTROL:
            request->buffer = &g_UsbCompositeDevPtr->audioUnified.maxTreble;
            request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.maxTreble);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_DELAY_CONTROL:
            request->buffer = g_UsbCompositeDevPtr->audioUnified.maxDelay;
            request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.maxDelay);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_VOLUME_CONTROL:
            request->buffer = g_UsbCompositeDevPtr->audioUnified.resVolume;
            request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.resVolume);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_BASS_CONTROL:
            request->buffer = &g_UsbCompositeDevPtr->audioUnified.resBass;
            request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.resBass);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_MID_CONTROL:
            request->buffer = &g_UsbCompositeDevPtr->audioUnified.resMid;
            request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.resMid);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_TREBLE_CONTROL:
            request->buffer = &g_UsbCompositeDevPtr->audioUnified.resTreble;
            request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.resTreble);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_DELAY_CONTROL:
            request->buffer = g_UsbCompositeDevPtr->audioUnified.resDelay;
            request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.resDelay);
            break;
#if USB_DEVICE_CONFIG_AUDIO_CLASS_2_0
        case USB_DEVICE_AUDIO_CS_GET_CUR_SAMPLING_FREQ_CONTROL:
            if (entity_id == USB_AUDIO_CONTROL_SPEAKER_CLOCK_SOURCE_ENTITY_ID)
            {
                request->buffer = (uint8_t *)&g_UsbCompositeDevPtr->audioUnified.curSpeakerSampleFrequency;
                request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.curSpeakerSampleFrequency);
            }
            else if (entity_id == USB_AUDIO_CONTROL_RECORDER_CLOCK_SOURCE_ENTITY_ID)
            {
                request->buffer = (uint8_t *)&g_UsbCompositeDevPtr->audioUnified.curRecorderSampleFrequency;
                request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.curRecorderSampleFrequency);
            }
            else
            {
                /* no action */
            }
            break;
        case USB_DEVICE_AUDIO_CS_SET_CUR_SAMPLING_FREQ_CONTROL:
            if (request->isSetup == 1U)
            {
                if (entity_id == USB_AUDIO_CONTROL_SPEAKER_CLOCK_SOURCE_ENTITY_ID)
                {
                    request->buffer = (uint8_t *)&g_UsbCompositeDevPtr->audioUnified.curSpeakerSampleFrequency;
                    request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.curSpeakerSampleFrequency);
                }
                else if (entity_id == USB_AUDIO_CONTROL_RECORDER_CLOCK_SOURCE_ENTITY_ID)
                {
                    request->buffer = (uint8_t *)&g_UsbCompositeDevPtr->audioUnified.curRecorderSampleFrequency;
                    request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.curRecorderSampleFrequency);
                }
                else
                {
                    /* no action */
                }
            }
            break;
        case USB_DEVICE_AUDIO_CS_GET_CUR_CLOCK_VALID_CONTROL:
            request->buffer = &g_UsbCompositeDevPtr->audioUnified.curClockValid;
            request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.curClockValid);
            break;
        case USB_DEVICE_AUDIO_CS_SET_CUR_CLOCK_VALID_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_UsbCompositeDevPtr->audioUnified.curClockValid;
                request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.curClockValid);
            }
            break;
        case USB_DEVICE_AUDIO_CS_GET_RANGE_SAMPLING_FREQ_CONTROL:
            if (USB_AUDIO_CONTROL_SPEAKER_CLOCK_SOURCE_ENTITY_ID == entity_id)
            {
                request->buffer = (uint8_t *)&g_UsbCompositeDevPtr->audioUnified.speakerFreqControlRange;
                request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.speakerFreqControlRange);
            }
            else if (USB_AUDIO_CONTROL_RECORDER_CLOCK_SOURCE_ENTITY_ID == entity_id)
            {
                request->buffer = (uint8_t *)&g_UsbCompositeDevPtr->audioUnified.recorderFreqControlRange;
                request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.recorderFreqControlRange);
            }
            else
            {
                /* no action */
            }
            break;
        case USB_DEVICE_AUDIO_FU_GET_RANGE_VOLUME_CONTROL:
            request->buffer = (uint8_t *)&g_UsbCompositeDevPtr->audioUnified.volumeControlRange;
            request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.volumeControlRange);
            break;
#else
        case USB_DEVICE_AUDIO_EP_GET_CUR_SAMPLING_FREQ_CONTROL:
            request->buffer = g_UsbCompositeDevPtr->audioUnified.curSamplingFrequency;
            request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.curSamplingFrequency);
            break;
        case USB_DEVICE_AUDIO_EP_GET_MIN_SAMPLING_FREQ_CONTROL:
            request->buffer = g_UsbCompositeDevPtr->audioUnified.minSamplingFrequency;
            request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.minSamplingFrequency);
            break;
        case USB_DEVICE_AUDIO_EP_GET_MAX_SAMPLING_FREQ_CONTROL:
            request->buffer = g_UsbCompositeDevPtr->audioUnified.maxSamplingFrequency;
            request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.maxSamplingFrequency);
            break;
        case USB_DEVICE_AUDIO_EP_GET_RES_SAMPLING_FREQ_CONTROL:
            request->buffer = g_UsbCompositeDevPtr->audioUnified.resSamplingFrequency;
            request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.resSamplingFrequency);
            break;
        case USB_DEVICE_AUDIO_EP_SET_CUR_SAMPLING_FREQ_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_UsbCompositeDevPtr->audioUnified.curSamplingFrequency;
                request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.curSamplingFrequency);
            }
            break;
        case USB_DEVICE_AUDIO_EP_SET_RES_SAMPLING_FREQ_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_UsbCompositeDevPtr->audioUnified.resSamplingFrequency;
                request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.resSamplingFrequency);
            }
            break;
        case USB_DEVICE_AUDIO_EP_SET_MAX_SAMPLING_FREQ_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_UsbCompositeDevPtr->audioUnified.maxSamplingFrequency;
                request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.maxSamplingFrequency);
            }
            break;
        case USB_DEVICE_AUDIO_EP_SET_MIN_SAMPLING_FREQ_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_UsbCompositeDevPtr->audioUnified.minSamplingFrequency;
                request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.minSamplingFrequency);
            }
            break;
#endif
        case USB_DEVICE_AUDIO_FU_SET_CUR_VOLUME_CONTROL:
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            if (request->isSetup == 1U)
            {
                if (USB_AUDIO_SPEAKER_CONTROL_FEATURE_UNIT_ID == entity_id)
                {
                    request->buffer = g_UsbCompositeDevPtr->audioUnified.curSpeakerVolume20;
                    request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.curSpeakerVolume20);
                }
                else if (USB_AUDIO_RECORDER_CONTROL_FEATURE_UNIT_ID == entity_id)
                {
                    request->buffer = g_UsbCompositeDevPtr->audioUnified.curMicrophoneVolume20;
                    request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.curMicrophoneVolume20);
                }
                else
                {
                    /* no action */
                }
            }
            else
            {
                if (USB_AUDIO_SPEAKER_CONTROL_FEATURE_UNIT_ID == entity_id)
                {
                    g_UsbCompositeDevPtr->audioUnified.codecSpeakerTask |= VOLUME_CHANGE_TASK;
                }
                else if (USB_AUDIO_RECORDER_CONTROL_FEATURE_UNIT_ID == entity_id)
                {
                    g_UsbCompositeDevPtr->audioUnified.codecMicrophoneTask |= VOLUME_CHANGE_TASK;
                }
                else
                {
                    /* no action */
                }
            }
#else
            if (request->isSetup == 1U)
            {
                if (USB_AUDIO_SPEAKER_CONTROL_FEATURE_UNIT_ID == entity_id)
                {
                    request->buffer = g_UsbCompositeDevPtr->audioUnified.curSpeakerVolume;
                    request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.curSpeakerVolume);
                }
                else if (USB_AUDIO_RECORDER_CONTROL_FEATURE_UNIT_ID == entity_id)
                {
                    request->buffer = g_UsbCompositeDevPtr->audioUnified.curMicrophoneVolume;
                    request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.curMicrophoneVolume);
                }
                else
                {
                    /* no action */
                }
            }
            else
            {
                if (USB_AUDIO_SPEAKER_CONTROL_FEATURE_UNIT_ID == entity_id)
                {
                    volume = (uint16_t)((uint16_t)g_UsbCompositeDevPtr->audioUnified.curSpeakerVolume[1] << 8U);
                    volume |= (uint8_t)(g_UsbCompositeDevPtr->audioUnified.curSpeakerVolume[0]);
                    g_UsbCompositeDevPtr->audioUnified.codecSpeakerTask |= VOLUME_CHANGE_TASK;
                }
                else if (USB_AUDIO_RECORDER_CONTROL_FEATURE_UNIT_ID == entity_id)
                {
                    volume = (uint16_t)((uint16_t)g_UsbCompositeDevPtr->audioUnified.curMicrophoneVolume[1] << 8U);
                    volume |= (uint8_t)(g_UsbCompositeDevPtr->audioUnified.curMicrophoneVolume[0]);
                    g_UsbCompositeDevPtr->audioUnified.codecMicrophoneTask |= VOLUME_CHANGE_TASK;
                }
                else
                {
                    /* no action */
                }
            }
#endif
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_MUTE_CONTROL:
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            if (request->isSetup == 1U)
            {
                if (USB_AUDIO_SPEAKER_CONTROL_FEATURE_UNIT_ID == entity_id)
                {
                    request->buffer = &g_UsbCompositeDevPtr->audioUnified.curSpeakerMute20;
                    request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.curSpeakerMute20);
                }
                else if (USB_AUDIO_RECORDER_CONTROL_FEATURE_UNIT_ID == entity_id)
                {
                    request->buffer = &g_UsbCompositeDevPtr->audioUnified.curMicrophoneMute20;
                    request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.curMicrophoneMute20);
                }
                else
                {
                    /* no action */
                }
            }
            else
            {
                /* trigger task, only adjust speaker mute/unmute practically */
                if (USB_AUDIO_SPEAKER_CONTROL_FEATURE_UNIT_ID == entity_id)
                {
                    if (g_UsbCompositeDevPtr->audioUnified.curSpeakerMute20)
                    {
                        g_UsbCompositeDevPtr->audioUnified.codecSpeakerTask |= MUTE_CODEC_TASK;
                    }
                    else
                    {
                        g_UsbCompositeDevPtr->audioUnified.codecSpeakerTask |= UNMUTE_CODEC_TASK;
                    }
                }
                else if (USB_AUDIO_RECORDER_CONTROL_FEATURE_UNIT_ID == entity_id)
                {
                    if (g_UsbCompositeDevPtr->audioUnified.curSpeakerMute20)
                    {
                        g_UsbCompositeDevPtr->audioUnified.codecMicrophoneTask |= MUTE_CODEC_TASK;
                    }
                    else
                    {
                        g_UsbCompositeDevPtr->audioUnified.codecMicrophoneTask |= UNMUTE_CODEC_TASK;
                    }
                }
                else
                {
                    /* no action */
                }
            }
#else
            if (request->isSetup == 1U)
            {
                if (USB_AUDIO_SPEAKER_CONTROL_FEATURE_UNIT_ID == entity_id)
                {
                    request->buffer = &g_UsbCompositeDevPtr->audioUnified.curSpeakerMute;
                    request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.curSpeakerMute);
                }
                else if (USB_AUDIO_RECORDER_CONTROL_FEATURE_UNIT_ID == entity_id)
                {
                    request->buffer = &g_UsbCompositeDevPtr->audioUnified.curMicrophoneMute;
                    request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.curMicrophoneMute);
                }
                else
                {
                    /* no action */
                }
            }
            else
            {
                /* trigger task, only adjust speaker mute/unmute practically */
                if (USB_AUDIO_SPEAKER_CONTROL_FEATURE_UNIT_ID == entity_id)
                {
                    if (g_UsbCompositeDevPtr->audioUnified.curSpeakerMute)
                    {
                        g_UsbCompositeDevPtr->audioUnified.codecSpeakerTask |= MUTE_CODEC_TASK;
                    }
                    else
                    {
                        g_UsbCompositeDevPtr->audioUnified.codecSpeakerTask |= UNMUTE_CODEC_TASK;
                    }
                }
                else if (USB_AUDIO_RECORDER_CONTROL_FEATURE_UNIT_ID == entity_id)
                {
                    if (g_UsbCompositeDevPtr->audioUnified.curMicrophoneMute)
                    {
                        g_UsbCompositeDevPtr->audioUnified.codecMicrophoneTask |= MUTE_CODEC_TASK;
                    }
                    else
                    {
                        g_UsbCompositeDevPtr->audioUnified.codecMicrophoneTask |= UNMUTE_CODEC_TASK;
                    }
                }
                else
                {
                    /* no action */
                }
            }
#endif
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_BASS_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_UsbCompositeDevPtr->audioUnified.curBass;
                request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.curBass);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_MID_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_UsbCompositeDevPtr->audioUnified.curMid;
                request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.curMid);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_TREBLE_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_UsbCompositeDevPtr->audioUnified.curTreble;
                request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.curTreble);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_AUTOMATIC_GAIN_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_UsbCompositeDevPtr->audioUnified.curAutomaticGain;
                request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.curAutomaticGain);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_DELAY_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_UsbCompositeDevPtr->audioUnified.curDelay;
                request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.curDelay);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_VOLUME_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_UsbCompositeDevPtr->audioUnified.minVolume;
                request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.minVolume);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_BASS_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_UsbCompositeDevPtr->audioUnified.minBass;
                request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.minBass);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_MID_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_UsbCompositeDevPtr->audioUnified.minMid;
                request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.minMid);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_TREBLE_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_UsbCompositeDevPtr->audioUnified.minTreble;
                request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.minTreble);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_DELAY_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_UsbCompositeDevPtr->audioUnified.minDelay;
                request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.minDelay);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_VOLUME_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_UsbCompositeDevPtr->audioUnified.maxVolume;
                request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.maxVolume);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_BASS_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_UsbCompositeDevPtr->audioUnified.maxBass;
                request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.maxBass);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_MID_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_UsbCompositeDevPtr->audioUnified.maxMid;
                request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.maxMid);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_TREBLE_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_UsbCompositeDevPtr->audioUnified.maxTreble;
                request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.maxTreble);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_DELAY_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_UsbCompositeDevPtr->audioUnified.maxDelay;
                request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.maxDelay);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_VOLUME_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_UsbCompositeDevPtr->audioUnified.resVolume;
                request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.resVolume);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_BASS_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_UsbCompositeDevPtr->audioUnified.resBass;
                request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.resBass);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_MID_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_UsbCompositeDevPtr->audioUnified.resMid;
                request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.resMid);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_TREBLE_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_UsbCompositeDevPtr->audioUnified.resTreble;
                request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.resTreble);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_DELAY_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_UsbCompositeDevPtr->audioUnified.resDelay;
                request->length = sizeof(g_UsbCompositeDevPtr->audioUnified.resDelay);
            }
            break;
        default:
            error = kStatus_USB_InvalidRequest;
            break;
    }
    return error;
}

#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
#else
void USB_DeviceCalculateFeedback(void)
{
    volatile static uint64_t totalFrameValue = 0U;
    volatile static PL_UINT32 frameDistance   = 0U;
    volatile static PL_UINT32 feedbackValue   = 0U;

    PL_UINT32 audioSpeakerUsedSpace = 0U;

    /* feedback interval is AUDIO_CALCULATE_Ff_INTERVAL */
    if (USB_SPEED_HIGH == g_UsbCompositeDevPtr->speed)
    {
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0) /* high speed, feedback interval is AUDIO_CALCULATE_Ff_INTERVAL ms */
        if (g_UsbCompositeDevPtr->audioUnified.speakerIntervalCount !=
            AUDIO_CALCULATE_Ff_INTERVAL *
                (AUDIO_OUT_TRANSFER_LENGTH_ONE_FRAME / g_UsbCompositeDevPtr->audioUnified.audioPlayTransferSize))
#else
        if (g_UsbCompositeDevPtr->audioUnified.speakerIntervalCount != AUDIO_CALCULATE_Ff_INTERVAL)
#endif
        {
            g_UsbCompositeDevPtr->audioUnified.speakerIntervalCount++;
            return;
        }
    }
    else /* full speed, feedback interval is AUDIO_CALCULATE_Ff_INTERVAL ms */
    {
        if (g_UsbCompositeDevPtr->audioUnified.speakerIntervalCount != AUDIO_CALCULATE_Ff_INTERVAL)
        {
            g_UsbCompositeDevPtr->audioUnified.speakerIntervalCount++;
            return;
        }
    }

    if (0U == g_UsbCompositeDevPtr->audioUnified.firstCalculateFeedback)
    {
        g_UsbCompositeDevPtr->audioUnified.speakerIntervalCount = 0;
        g_UsbCompositeDevPtr->audioUnified.currentFrameCount    = 0;
        g_UsbCompositeDevPtr->audioUnified.audioSendCount[0]    = 0;
        g_UsbCompositeDevPtr->audioUnified.audioSendCount[1]    = 0;
        totalFrameValue                                           = 0;
        frameDistance                                             = 0;
        feedbackValue                                             = 0;
        USB_DeviceClassGetCurrentFrameCount(CONTROLLER_ID,
                                            (PL_UINT32 *)&g_UsbCompositeDevPtr->audioUnified.lastFrameCount);
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
#if (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
        if (USB_SPEED_HIGH == g_UsbCompositeDevPtr->speed)
        {
            g_UsbCompositeDevPtr->audioUnified.lastFrameCount += s_microFrameCountIp3511HS;
        }
#endif
#endif
        g_UsbCompositeDevPtr->audioUnified.firstCalculateFeedback = 1U;
        return;
    }
    g_UsbCompositeDevPtr->audioUnified.speakerIntervalCount = 0;
    USB_DeviceClassGetCurrentFrameCount(CONTROLLER_ID,
                                        (PL_UINT32 *)&g_UsbCompositeDevPtr->audioUnified.currentFrameCount);
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
#if (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
    if (USB_SPEED_HIGH == g_UsbCompositeDevPtr->speed)
    {
        g_UsbCompositeDevPtr->audioUnified.currentFrameCount += s_microFrameCountIp3511HS;
    }
#endif
#endif
    frameDistance = ((g_UsbCompositeDevPtr->audioUnified.currentFrameCount + USB_DEVICE_MAX_FRAME_COUNT + 1U -
                      g_UsbCompositeDevPtr->audioUnified.lastFrameCount) &
                     USB_DEVICE_MAX_FRAME_COUNT);
    g_UsbCompositeDevPtr->audioUnified.lastFrameCount = g_UsbCompositeDevPtr->audioUnified.currentFrameCount;

    totalFrameValue += frameDistance;

    if (1U == g_UsbCompositeDevPtr->audioUnified.stopFeedbackUpdate)
    {
        return;
    }

    if (1U == g_UsbCompositeDevPtr->audioUnified.feedbackDiscardFlag)
    {
        if (0 != g_UsbCompositeDevPtr->audioUnified.feedbackDiscardTimes)
        {
            g_UsbCompositeDevPtr->audioUnified.feedbackDiscardTimes--;
            if (0 != g_UsbCompositeDevPtr->audioUnified.lastFeedbackValue)
            {
                AUDIO_UPDATE_FEEDBACK_DATA(audioFeedBackBuffer, g_UsbCompositeDevPtr->audioUnified.lastFeedbackValue);
            }
            return;
        }
        else
        {
            g_UsbCompositeDevPtr->audioUnified.feedbackDiscardFlag = 0;
        }
    }

    if (USB_SPEED_HIGH == g_UsbCompositeDevPtr->speed)
    {
        feedbackValue = (PL_UINT32)((((((uint64_t)g_UsbCompositeDevPtr->audioUnified.audioSendCount[1]) << 32U) |
                                     g_UsbCompositeDevPtr->audioUnified.audioSendCount[0])) *
                                   1024UL * 8UL / totalFrameValue /
                                   ((uint64_t)(AUDIO_OUT_FORMAT_CHANNELS * AUDIO_OUT_FORMAT_SIZE)));
    }
    else
    {
        feedbackValue =
            (PL_UINT32)((((((uint64_t)g_UsbCompositeDevPtr->audioUnified.audioSendCount[1]) << 32U) |
                         g_UsbCompositeDevPtr->audioUnified.audioSendCount[0])) *
                       1024UL / totalFrameValue / ((uint64_t)(AUDIO_OUT_FORMAT_CHANNELS * AUDIO_OUT_FORMAT_SIZE)));
    }

    audioSpeakerUsedSpace = USB_AudioSpeakerBufferSpaceUsed();
    if (audioSpeakerUsedSpace <= (g_UsbCompositeDevPtr->audioUnified.audioPlayTransferSize *
                                  USB_AUDIO_PLAY_BUFFER_FEEDBACK_TOLERANCE_THRESHOLD))
    {
        feedbackValue += AUDIO_ADJUST_MIN_STEP;
    }

    if ((audioSpeakerUsedSpace + (g_UsbCompositeDevPtr->audioUnified.audioPlayTransferSize *
                                  USB_AUDIO_PLAY_BUFFER_FEEDBACK_TOLERANCE_THRESHOLD)) >=
        g_UsbCompositeDevPtr->audioUnified.audioPlayBufferSize)
    {
        feedbackValue -= AUDIO_ADJUST_MIN_STEP;
    }

    g_UsbCompositeDevPtr->audioUnified.lastFeedbackValue = feedbackValue;
    AUDIO_UPDATE_FEEDBACK_DATA(audioFeedBackBuffer, feedbackValue);
}

/* The USB_RecorderDataMatch() function increase/decrease the adjusted packet interval according to the reserved
 * ringbuffer size */
PL_UINT32 USB_RecorderDataMatch(PL_UINT32 reservedspace)
{
    PL_UINT32 epPacketSize = 0;
    if (reservedspace >=
        AUDIO_BUFFER_UPPER_LIMIT(AUDIO_RECORDER_DATA_WHOLE_BUFFER_COUNT_NORMAL * FS_ISO_IN_ENDP_PACKET_SIZE))
    {
        epPacketSize = FS_ISO_IN_ENDP_PACKET_SIZE + AUDIO_IN_FORMAT_SIZE * AUDIO_IN_FORMAT_CHANNELS;
    }
    else if ((reservedspace >=
              AUDIO_BUFFER_LOWER_LIMIT(AUDIO_RECORDER_DATA_WHOLE_BUFFER_COUNT_NORMAL * FS_ISO_IN_ENDP_PACKET_SIZE)) &&
             (reservedspace <
              AUDIO_BUFFER_UPPER_LIMIT(AUDIO_RECORDER_DATA_WHOLE_BUFFER_COUNT_NORMAL * FS_ISO_IN_ENDP_PACKET_SIZE)))
    {
        epPacketSize = FS_ISO_IN_ENDP_PACKET_SIZE;
    }
    else if (reservedspace <
             AUDIO_BUFFER_LOWER_LIMIT(AUDIO_RECORDER_DATA_WHOLE_BUFFER_COUNT_NORMAL * FS_ISO_IN_ENDP_PACKET_SIZE))
    {
        epPacketSize = FS_ISO_IN_ENDP_PACKET_SIZE - AUDIO_IN_FORMAT_SIZE * AUDIO_IN_FORMAT_CHANNELS;
    }
    else
    {
    }
    return epPacketSize;
}
#endif

/*!
 * @brief device Audio callback function.
 *
 * This function handle the Audio class specified event.
 * @param handle          The USB class  handle.
 * @param event           The USB device event type.
 * @param param           The parameter of the class specific event.
 * @return kStatus_USB_Success or error.
 */
__attribute__((__section__(".ramfunc*")))
usb_status_t USB_DeviceAudioCompositeCallback(class_handle_t handle, PL_UINT32 event, void *param)
{
    usb_status_t error = kStatus_USB_InvalidRequest;
    usb_device_endpoint_callback_message_struct_t *ep_cb_param;
    ep_cb_param           = (usb_device_endpoint_callback_message_struct_t *)param;
    PL_UINT32 epPacketSize = g_UsbCompositeDevPtr->FS_ISO_IN_ENDP_PACKET_SIZE;
    switch (event)
    {
        case kUSB_DeviceAudioEventStreamSendResponse:
        	 if ((g_UsbCompositeDevPtr->audioUnified.attach) && (ep_cb_param->length != (USB_CANCELLED_TRANSFER_LENGTH)))
			{

				if (g_appHandle.UsbCompositeDev_handle.audioUnified.startRec  == 0)
				{
					g_appHandle.UsbCompositeDev_handle.audioUnified.startRec =1;
				}

				if (g_appHandle.UsbCompositeDev_handle.cirBuffTx.flagMidFull ==1)
				{

					toolsCircularBufferRead_I_I(&g_appHandle.UsbCompositeDev_handle.cirBuffTx,
												&audioRecPacket[0],
												g_UsbCompositeDevPtr->FS_ISO_IN_ENDP_PACKET_SIZE);

					error = USB_DeviceAudioSend( g_appHandle.UsbCompositeDev_handle.audioUnified.audioRecorderHandle,		// USB handle
												 USB_AUDIO_RECORDER_STREAM_ENDPOINT,
												 &audioRecPacket[0],														// source,  local buffer start address
												 g_UsbCompositeDevPtr->FS_ISO_IN_ENDP_PACKET_SIZE);							// size in byte
				}
				else
				{
					// Not enough samples in Circular buffer, so we can send 0
					memset(&audioRecPacket[0],
						   0x0,
						   g_UsbCompositeDevPtr->FS_ISO_IN_ENDP_PACKET_SIZE);

					error = USB_DeviceAudioSend( g_appHandle.UsbCompositeDev_handle.audioUnified.audioRecorderHandle,		// USB handle
												 USB_AUDIO_RECORDER_STREAM_ENDPOINT,
												 &audioRecPacket[0],														// source,  local buffer start address
												 g_UsbCompositeDevPtr->FS_ISO_IN_ENDP_PACKET_SIZE);							// size in byte
				}

			}

            break;
#if RX_PATH_PRESENT
        case kUSB_DeviceAudioEventStreamRecvResponse:
            /* endpoint callback length is USB_CANCELLED_TRANSFER_LENGTH (0xFFFFFFFFU) when transfer is canceled */
            if ((g_UsbCompositeDevPtr->audioUnified.attach) && (ep_cb_param->length != (USB_CANCELLED_TRANSFER_LENGTH)))
            {
            	error = USB_DeviceAudioRecv( handle,
											 USB_AUDIO_SPEAKER_STREAM_ENDPOINT,
											 &audioPlayPacket[0],
											 g_UsbCompositeDevPtr->audioUnified.currentStreamOutMaxPacketSize);
				// if Circular buffer midfull, raise up the midFull flag
				if (g_appHandle.UsbCompositeDev_handle.cirBuffRx.flagMidFull ==0)
				{
					if (toolsCircularBufferReadableData(&g_appHandle.UsbCompositeDev_handle.cirBuffRx) > (g_appHandle.UsbCompositeDev_handle.cirBuffRx.cbSize/2))
					{
						g_appHandle.UsbCompositeDev_handle.cirBuffRx.flagMidFull = 1;
					}
				}
				g_UsbCompositeDevPtr->audioUnified.usbRecvCount += ep_cb_param->length;
				g_UsbCompositeDevPtr->audioUnified.usbRecvTimes++;
				// Write Interleaved data from audioPlayPacket to Circular buffer RX
 				toolsCircularBufferWrite_I_I(&g_appHandle.UsbCompositeDev_handle.cirBuffRx,
 											 &audioPlayPacket[0],
											 ep_cb_param->length);
            }

            break;
#endif
        default:
            if (param && (event > 0xFF))
            {
                error = USB_DeviceAudioRequest(handle, event, param);
            }
            break;
    }
    return error;
}

#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
#if (defined USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS)
__attribute__((__section__(".ramfunc*")))
void CTIMER_SOF_TOGGLE_HANDLER_FRO(PL_UINT32 i)
{
    PL_UINT32 currentCtCap = 0, pllCountPeriod = 0;
    PL_UINT32 usedSpace      = 0;
    static int32_t pllCount = 0, pllDiff = 0;
    static int32_t err, abs_err;
    PL_UINT32 up_change                 = 0;
    PL_UINT32 down_change               = 0;
    static PL_UINT32 delay_adj_up       = 0;
    static PL_UINT32 delay_adj_down     = 0;
    static PL_UINT32 FroPreUsbRecvCount = 0U;



    if (CTIMER_GetStatusFlags(CTIMER1) & (1 << 4U))
    {
        /* Clear interrupt flag.*/
        CTIMER_ClearStatusFlags(CTIMER1, (1 << 4U));
    }

    if (g_appHandle.UsbCompositeDev_handle.audioUnified.froTrimIntervalCount != AUDIO_FRO_ADJUST_INTERVAL)
    {
        g_appHandle.UsbCompositeDev_handle.audioUnified.froTrimIntervalCount++;
        return;
    }

    g_appHandle.UsbCompositeDev_handle.audioUnified.froTrimIntervalCount = 1;
    currentCtCap                                  = CTIMER1->CR[0];
    pllCountPeriod                                = currentCtCap - g_appHandle.UsbCompositeDev_handle.audioUnified.usbFroTicksPrev;
    g_appHandle.UsbCompositeDev_handle.audioUnified.usbFroTicksPrev      = currentCtCap;
    pllCount                                      = pllCountPeriod;

    if (g_appHandle.UsbCompositeDev_handle.audioUnified.attach)
    {
        if (abs(pllCount - AUDIO_FRO_USB_SOF_INTERVAL_TICK_COUNT) < AUDIO_FRO_USB_SOF_INTERVAL_VALID_DEVIATION)
        {
            pllDiff = pllCount - g_appHandle.UsbCompositeDev_handle.audioUnified.usbFroTicksEma;
            g_appHandle.UsbCompositeDev_handle.audioUnified.usbFroTickEmaFrac += (pllDiff % 8);
            g_appHandle.UsbCompositeDev_handle.audioUnified.usbFroTicksEma += (pllDiff / 8) + g_appHandle.UsbCompositeDev_handle.audioUnified.usbFroTickEmaFrac / 8;
            g_appHandle.UsbCompositeDev_handle.audioUnified.usbFroTickEmaFrac = (g_appHandle.UsbCompositeDev_handle.audioUnified.usbFroTickEmaFrac % 8);

            err     = g_appHandle.UsbCompositeDev_handle.audioUnified.usbFroTicksEma - AUDIO_FRO_USB_SOF_INTERVAL_TICK_COUNT;
            abs_err = abs(err);
            if (abs_err >= g_appHandle.UsbCompositeDev_handle.audioUnified.usbFroTickBasedPrecision)
            {
                if (err > 0)
                {
                    down_change = 1;
                }
                else
                {
                    up_change = 1;
                }
            }

            if (g_appHandle.UsbCompositeDev_handle.audioUnified.startPlayFlag)
            {
                /* if USB transfer stops, can not use data length to do adjustment */
                if (0U == g_appHandle.UsbCompositeDev_handle.audioUnified.stopDataLengthAudioAdjust)
                {
                    /* USB is transferring */
                    if (FroPreUsbRecvCount != g_appHandle.UsbCompositeDev_handle.audioUnified.usbRecvCount)
                    {
                        FroPreUsbRecvCount = g_appHandle.UsbCompositeDev_handle.audioUnified.usbRecvCount;
                        usedSpace          = USB_AudioSpeakerBufferSpaceUsed();
                        if ((usedSpace + (g_appHandle.UsbCompositeDev_handle.audioUnified.audioPlayTransferSize *
                                          AUDIO_SYNC_DATA_BASED_ADJUST_THRESHOLD)) >=
                            g_appHandle.UsbCompositeDev_handle.audioUnified.audioPlayBufferSize)
                        {
                            if (delay_adj_up == 0)
                            {
                                delay_adj_down = 0;
                                delay_adj_up   = AUDIO_FRO_TRIM_DATA_BASED_INTERVAL;
                                up_change      = 1;
                            }
                            else
                            {
                                delay_adj_up--;
                            }
                        }
                        else if (usedSpace <= (g_appHandle.UsbCompositeDev_handle.audioUnified.audioPlayTransferSize *
                                               AUDIO_SYNC_DATA_BASED_ADJUST_THRESHOLD))
                        {
                            if (delay_adj_down == 0)
                            {
                                delay_adj_up   = 0;
                                delay_adj_down = AUDIO_FRO_TRIM_DATA_BASED_INTERVAL;
                                down_change    = 1;
                            }
                            else
                            {
                                delay_adj_down--;
                            }
                        }
                        else
                        {
                            /* no action */
                        }
                    }
                }
            }
        }

        if (down_change)
        {
            audio_fro_trim_down();
            //down_change=0;
        }
        if (up_change)
        {
            audio_fro_trim_up();
            //up_change=0;
        }
    }

}
#endif /* USB_DEVICE_CONFIG_LPCIP3511FS */
__attribute__((__section__(".ramfunc*")))
void CTIMER_SOF_TOGGLE_HANDLER_PLL(PL_UINT32 i)
{
    PL_UINT32 currentCtCap = 0, pllCountPeriod = 0, pll_change = 0;
    PL_UINT32 usedSpace      = 0;
    static int32_t pllCount = 0, pllDiff = 0;
    static int32_t err, abs_err;
    static PL_UINT32 delay_adj_up       = 0;
    static PL_UINT32 delay_adj_down     = 0;
    static PL_UINT32 PllPreUsbRecvCount = 0U;


    if (CTIMER_GetStatusFlags(CTIMER0) & (1 << 4U))
    {
        /* Clear interrupt flag.*/
        CTIMER_ClearStatusFlags(CTIMER0, (1 << 4U));
    }

    if (g_appHandle.UsbCompositeDev_handle.audioUnified.speakerIntervalCount != AUDIO_PLL_ADJUST_INTERVAL)
    {
        g_appHandle.UsbCompositeDev_handle.audioUnified.speakerIntervalCount++;
        return;
    }
    g_appHandle.UsbCompositeDev_handle.audioUnified.speakerIntervalCount = 1;
    currentCtCap                                  = CTIMER0->CR[0];
    pllCountPeriod                                = currentCtCap - g_appHandle.UsbCompositeDev_handle.audioUnified.audioPllTicksPrev;
    g_appHandle.UsbCompositeDev_handle.audioUnified.audioPllTicksPrev    = currentCtCap;
    pllCount                                      = pllCountPeriod;
    if (g_appHandle.UsbCompositeDev_handle.audioUnified.attach)
    {
        if (abs(pllCount - AUDIO_PLL_USB_SOF_INTERVAL_TICK_COUNT) < AUDIO_PLL_USB_SOF_INTERVAL_VALID_DEVIATION)
        {
            pllDiff = pllCount - g_appHandle.UsbCompositeDev_handle.audioUnified.audioPllTicksEma;
            g_appHandle.UsbCompositeDev_handle.audioUnified.audioPllTickEmaFrac += (pllDiff % 8);
            g_appHandle.UsbCompositeDev_handle.audioUnified.audioPllTicksEma +=
                (pllDiff / 8) + g_appHandle.UsbCompositeDev_handle.audioUnified.audioPllTickEmaFrac / 8;
            g_appHandle.UsbCompositeDev_handle.audioUnified.audioPllTickEmaFrac = (g_appHandle.UsbCompositeDev_handle.audioUnified.audioPllTickEmaFrac % 8);

            err     = g_appHandle.UsbCompositeDev_handle.audioUnified.audioPllTicksEma - AUDIO_PLL_USB_SOF_INTERVAL_TICK_COUNT;
            abs_err = abs(err);
            g_appHandle.UsbCompositeDev_handle.audioUnified.audioPllTicksDiff=err;

            if (abs_err)		//Stan changed --- even abs_err is only 1, it is still necessary to adjust the PLL
            {
                if (err > 0)
                {
                    g_appHandle.UsbCompositeDev_handle.audioUnified.curAudioPllFrac -= abs_err * 4;
                }
                else
                {
                    g_appHandle.UsbCompositeDev_handle.audioUnified.curAudioPllFrac += abs_err * 4;
                }
                pll_change = 1;
            }

        }

        if (pll_change)
        {
            USB_AudioPllChange();
        }
    }
}
#endif /* USB_DEVICE_AUDIO_USE_SYNC_MODE */

/* The USB_DeviceAudioRecorderStatusReset() function resets the audio recorder status to the initialized status */
void USB_DeviceAudioRecorderStatusReset(void)
{
    g_UsbCompositeDevPtr->audioUnified.startRec              = 0;
    g_UsbCompositeDevPtr->audioUnified.startRecHalfFull      = 0;
    g_UsbCompositeDevPtr->audioUnified.audioRecvCount        = 0;
    g_UsbCompositeDevPtr->audioUnified.usbSendTimes          = 0;
    g_UsbCompositeDevPtr->audioUnified.tdWriteNumberRec      = 0;
    g_UsbCompositeDevPtr->audioUnified.tdReadNumberRec       = 0;
    g_UsbCompositeDevPtr->audioUnified.recorderReservedSpace = 0;
}

/* The USB_DeviceAudioSpeakerStatusReset() function resets the audio speaker status to the initialized status */
void USB_DeviceAudioSpeakerStatusReset(void)
{
    g_UsbCompositeDevPtr->audioUnified.startPlayFlag                 = 0;
    g_UsbCompositeDevPtr->audioUnified.startPlayHalfFull             = 0;
    g_UsbCompositeDevPtr->audioUnified.tdReadNumberPlay              = 0;
    g_UsbCompositeDevPtr->audioUnified.tdWriteNumberPlay             = 0;
    g_UsbCompositeDevPtr->audioUnified.audioSpeakerReadDataCount[0]  = 0;
    g_UsbCompositeDevPtr->audioUnified.audioSpeakerReadDataCount[1]  = 0;
    g_UsbCompositeDevPtr->audioUnified.audioSpeakerWriteDataCount[0] = 0;
    g_UsbCompositeDevPtr->audioUnified.audioSpeakerWriteDataCount[1] = 0;
    g_UsbCompositeDevPtr->audioUnified.audioSendCount[0]             = 0;
    g_UsbCompositeDevPtr->audioUnified.audioSendCount[1]             = 0;
    g_UsbCompositeDevPtr->audioUnified.usbRecvCount                  = 0;
    g_UsbCompositeDevPtr->audioUnified.lastAudioSendCount            = 0;
    g_UsbCompositeDevPtr->audioUnified.audioSendTimes                = 0;
    g_UsbCompositeDevPtr->audioUnified.usbRecvTimes                  = 0;
    g_UsbCompositeDevPtr->audioUnified.speakerIntervalCount          = 0;
    g_UsbCompositeDevPtr->audioUnified.speakerReservedSpace          = 0;
    g_UsbCompositeDevPtr->audioUnified.timesFeedbackCalculate        = 0;
    g_UsbCompositeDevPtr->audioUnified.speakerDetachOrNoInput        = 0;
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
    g_UsbCompositeDevPtr->audioUnified.audioPllTicksPrev = 0U;
#if defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
    g_UsbCompositeDevPtr->audioUnified.usbFroTicksPrev          = 0U;
    g_UsbCompositeDevPtr->audioUnified.usbFroTicksEma           = AUDIO_FRO_USB_SOF_INTERVAL_TICK_COUNT;
    g_UsbCompositeDevPtr->audioUnified.usbFroTickEmaFrac        = 0U;
    g_UsbCompositeDevPtr->audioUnified.usbFroTickBasedPrecision = AUDIO_USB_FRO_TRIM_TICK_BASED_PRECISION;
#endif
    g_UsbCompositeDevPtr->audioUnified.audioPllTicksEma           = AUDIO_PLL_USB_SOF_INTERVAL_TICK_COUNT;
    g_UsbCompositeDevPtr->audioUnified.audioPllTickEmaFrac        = 0U;
    g_UsbCompositeDevPtr->audioUnified.audioPllTickBasedPrecision = AUDIO_PLL_FRACTION_TICK_BASED_PRECISION;
    g_UsbCompositeDevPtr->audioUnified.stopDataLengthAudioAdjust  = 0U;
#else
    g_UsbCompositeDevPtr->audioUnified.firstCalculateFeedback = 0;
    g_UsbCompositeDevPtr->audioUnified.lastFrameCount         = 0;
    g_UsbCompositeDevPtr->audioUnified.currentFrameCount      = 0;
    g_UsbCompositeDevPtr->audioUnified.feedbackDiscardFlag    = 0U;
    g_UsbCompositeDevPtr->audioUnified.feedbackDiscardTimes   = AUDIO_SPEAKER_FEEDBACK_DISCARD_COUNT;

    /* use the last saved feedback value */
    if (g_UsbCompositeDevPtr->audioUnified.lastFeedbackValue)
    {
        AUDIO_UPDATE_FEEDBACK_DATA(audioFeedBackBuffer, g_UsbCompositeDevPtr->audioUnified.lastFeedbackValue);
    }
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
usb_status_t USB_DeviceAudioCompositeSetConfigure(class_handle_t handle, uint8_t configure)
{
    if (USB_COMPOSITE_CONFIGURE_INDEX == configure)
    {
        g_UsbCompositeDevPtr->audioUnified.attach = 1U;
    }
    return kStatus_USB_Success;
}

usb_status_t USB_DeviceAudioRecorderSetInterface(class_handle_t handle, uint8_t interface, uint8_t alternateSetting)
{
    usb_status_t error = kStatus_USB_Success;

    if (alternateSetting == USB_AUDIO_RECORDER_STREAM_INTERFACE_ALTERNATE_1)
    {
        USB_DeviceAudioRecorderStatusReset();
        error =
            USB_DeviceAudioSend(g_UsbCompositeDevPtr->audioUnified.audioRecorderHandle,
                                USB_AUDIO_RECORDER_STREAM_ENDPOINT, &audioRecPacket[0], g_UsbCompositeDevPtr->FS_ISO_IN_ENDP_PACKET_SIZE);
    }

    //if (alternateSetting == USB_AUDIO_RECORDER_STREAM_INTERFACE_ALTERNATE_0)
    //{
        USB_DeviceAudioRecorderStatusReset();
    //}
    return error;
}

usb_status_t USB_DeviceAudioSpeakerSetInterface(class_handle_t handle, uint8_t interface, uint8_t alternateSetting)
{
    usb_status_t error = kStatus_USB_Success;

    if (USB_AUDIO_SPEAKER_STREAM_INTERFACE_ALTERNATE_1 == alternateSetting)
    {
        error =
            USB_DeviceAudioRecv(g_UsbCompositeDevPtr->audioUnified.audioSpeakerHandle,
                                USB_AUDIO_SPEAKER_STREAM_ENDPOINT, &audioPlayPacket[0], g_UsbCompositeDevPtr->FS_ISO_OUT_ENDP_PACKET_SIZE);
        if (error != kStatus_USB_Success)
        {
            return error;
        }
        else
        {
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
#else
            if (!feedbackValueUpdating)
            {
                *((PL_UINT32 *)&usbAudioFeedBackBuffer[0]) = *((PL_UINT32 *)&audioFeedBackBuffer[0]);
            }
            error = USB_DeviceAudioSend(g_UsbCompositeDevPtr->audioUnified.audioSpeakerHandle,
                                        USB_AUDIO_SPEAKER_FEEDBACK_ENDPOINT, usbAudioFeedBackBuffer,
                                        (USB_SPEED_HIGH == g_appHandle.UsbCompositeDev_handle.speed) ? HS_ISO_FEEDBACK_ENDP_PACKET_SIZE :
                                                                                FS_ISO_FEEDBACK_ENDP_PACKET_SIZE);
#endif
        }
    }

    //if (USB_AUDIO_SPEAKER_STREAM_INTERFACE_ALTERNATE_0 == alternateSetting)
    //{
    	USB_DeviceAudioSpeakerStatusReset();
    //}

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

	if((device_composite->AUDIO_IN_SAMPLING_RATE_KHZ!=16)&&(device_composite->AUDIO_IN_SAMPLING_RATE_KHZ!=48)&&(device_composite->AUDIO_IN_SAMPLING_RATE_KHZ!=32))
		return kStatus_USB_Error;
	if((device_composite->AUDIO_OUT_SAMPLING_RATE_KHZ!=16)&&(device_composite->AUDIO_OUT_SAMPLING_RATE_KHZ!=48)&&(device_composite->AUDIO_OUT_SAMPLING_RATE_KHZ!=32))		//upstream can only be 16KHz --- if do 48KHz, there is no enough memory to hold 4ch 32bit 32ms upstreaming buffer
		return kStatus_USB_Error;
	if(device_composite->AUDIO_IN_SAMPLING_RATE_KHZ!=device_composite->AUDIO_OUT_SAMPLING_RATE_KHZ)
		return kStatus_USB_Error;
	if(device_composite->AUDIO_IN_FORMAT_CHANNELS>5)
		return kStatus_USB_Error;
	if((device_composite->AUDIO_OUT_FORMAT_CHANNELS!=1)&&(device_composite->AUDIO_OUT_FORMAT_CHANNELS!=2))
		return kStatus_USB_Error;
	if((device_composite->AUDIO_IN_FORMAT_SIZE!=4)&&(device_composite->AUDIO_IN_FORMAT_SIZE!=2))
		return kStatus_USB_Error;
	if((device_composite->AUDIO_OUT_FORMAT_SIZE!=4)&&(device_composite->AUDIO_OUT_FORMAT_SIZE!=2))
		return kStatus_USB_Error;


#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
    CTIMER_CaptureInit();
#else
    AUDIO_UPDATE_FEEDBACK_DATA(audioFeedBackBuffer, AUDIO_SAMPLING_RATE_TO_10_14);
#endif
    g_UsbCompositeDevPtr                                       = device_composite;
    g_UsbCompositeDevPtr->audioUnified.copyProtect             = 0x01U;
    g_UsbCompositeDevPtr->audioUnified.curSpeakerMute          = 0x00U;
    g_UsbCompositeDevPtr->audioUnified.curMicrophoneMute       = 0x00U;
    g_UsbCompositeDevPtr->audioUnified.curSpeakerVolume[0]     = 0x00U;
    g_UsbCompositeDevPtr->audioUnified.curSpeakerVolume[1]     = 0x1fU;
    g_UsbCompositeDevPtr->audioUnified.curMicrophoneVolume[0]  = 0x00U;
    g_UsbCompositeDevPtr->audioUnified.curMicrophoneVolume[1]  = 0x1fU;
    g_UsbCompositeDevPtr->audioUnified.minVolume[0]            = 0x00U;
    g_UsbCompositeDevPtr->audioUnified.minVolume[1]            = 0x00U;
    g_UsbCompositeDevPtr->audioUnified.maxVolume[0]            = 0x00U;
    g_UsbCompositeDevPtr->audioUnified.maxVolume[1]            = 0X43U;
    g_UsbCompositeDevPtr->audioUnified.resVolume[0]            = 0x01U;
    g_UsbCompositeDevPtr->audioUnified.resVolume[1]            = 0x00U;
    g_UsbCompositeDevPtr->audioUnified.curBass                 = 0x00U;
    g_UsbCompositeDevPtr->audioUnified.curBass                 = 0x00U;
    g_UsbCompositeDevPtr->audioUnified.minBass                 = 0x80U;
    g_UsbCompositeDevPtr->audioUnified.maxBass                 = 0x7FU;
    g_UsbCompositeDevPtr->audioUnified.resBass                 = 0x01U;
    g_UsbCompositeDevPtr->audioUnified.curMid                  = 0x00U;
    g_UsbCompositeDevPtr->audioUnified.minMid                  = 0x80U;
    g_UsbCompositeDevPtr->audioUnified.maxMid                  = 0x7FU;
    g_UsbCompositeDevPtr->audioUnified.resMid                  = 0x01U;
    g_UsbCompositeDevPtr->audioUnified.curTreble               = 0x01U;
    g_UsbCompositeDevPtr->audioUnified.minTreble               = 0x80U;
    g_UsbCompositeDevPtr->audioUnified.maxTreble               = 0x7FU;
    g_UsbCompositeDevPtr->audioUnified.resTreble               = 0x01U;
    g_UsbCompositeDevPtr->audioUnified.curAutomaticGain        = 0x01U;
    g_UsbCompositeDevPtr->audioUnified.curDelay[0]             = 0x00U;
    g_UsbCompositeDevPtr->audioUnified.curDelay[1]             = 0x40U;
    g_UsbCompositeDevPtr->audioUnified.minDelay[0]             = 0x00U;
    g_UsbCompositeDevPtr->audioUnified.minDelay[1]             = 0x00U;
    g_UsbCompositeDevPtr->audioUnified.maxDelay[0]             = 0xFFU;
    g_UsbCompositeDevPtr->audioUnified.maxDelay[1]             = 0xFFU;
    g_UsbCompositeDevPtr->audioUnified.resDelay[0]             = 0x00U;
    g_UsbCompositeDevPtr->audioUnified.resDelay[1]             = 0x01U;
    g_UsbCompositeDevPtr->audioUnified.curLoudness             = 0x01U;
    g_UsbCompositeDevPtr->audioUnified.curSamplingFrequency[0] = 0x00U;
    g_UsbCompositeDevPtr->audioUnified.curSamplingFrequency[1] = 0x00U;
    g_UsbCompositeDevPtr->audioUnified.curSamplingFrequency[2] = 0x01U;
    g_UsbCompositeDevPtr->audioUnified.minSamplingFrequency[0] = 0x00U;
    g_UsbCompositeDevPtr->audioUnified.minSamplingFrequency[1] = 0x00U;
    g_UsbCompositeDevPtr->audioUnified.minSamplingFrequency[2] = 0x01U;
    g_UsbCompositeDevPtr->audioUnified.maxSamplingFrequency[0] = 0x00U;
    g_UsbCompositeDevPtr->audioUnified.maxSamplingFrequency[1] = 0x00U;
    g_UsbCompositeDevPtr->audioUnified.maxSamplingFrequency[2] = 0x01U;
    g_UsbCompositeDevPtr->audioUnified.resSamplingFrequency[0] = 0x00U;
    g_UsbCompositeDevPtr->audioUnified.resSamplingFrequency[1] = 0x00U;
    g_UsbCompositeDevPtr->audioUnified.resSamplingFrequency[2] = 0x01U;
#if USB_DEVICE_CONFIG_AUDIO_CLASS_2_0
    g_UsbCompositeDevPtr->audioUnified.curSpeakerMute20         = 0U;
    g_UsbCompositeDevPtr->audioUnified.curMicrophoneMute20      = 0U;
    g_UsbCompositeDevPtr->audioUnified.curClockValid            = 1U;
    g_UsbCompositeDevPtr->audioUnified.curSpeakerVolume20[0]    = 0x00U;
    g_UsbCompositeDevPtr->audioUnified.curSpeakerVolume20[1]    = 0x1FU;
    g_UsbCompositeDevPtr->audioUnified.curMicrophoneVolume20[0] = 0x00U;
    g_UsbCompositeDevPtr->audioUnified.curMicrophoneVolume20[1] = 0x1FU;

    g_UsbCompositeDevPtr->audioUnified.curSpeakerSampleFrequency             = (g_UsbCompositeDevPtr->AUDIO_OUT_SAMPLING_RATE_KHZ*1000);
    g_UsbCompositeDevPtr->audioUnified.speakerFreqControlRange.wNumSubRanges = 1U;
    g_UsbCompositeDevPtr->audioUnified.speakerFreqControlRange.wMIN          = (g_UsbCompositeDevPtr->AUDIO_OUT_SAMPLING_RATE_KHZ*1000);
    g_UsbCompositeDevPtr->audioUnified.speakerFreqControlRange.wMAX          = (g_UsbCompositeDevPtr->AUDIO_OUT_SAMPLING_RATE_KHZ*1000);
    g_UsbCompositeDevPtr->audioUnified.speakerFreqControlRange.wRES          = 0U;

    g_UsbCompositeDevPtr->audioUnified.curRecorderSampleFrequency             = (g_UsbCompositeDevPtr->AUDIO_IN_SAMPLING_RATE_KHZ*1000);
    g_UsbCompositeDevPtr->audioUnified.recorderFreqControlRange.wNumSubRanges = 1U;
    g_UsbCompositeDevPtr->audioUnified.recorderFreqControlRange.wMIN          = (g_UsbCompositeDevPtr->AUDIO_IN_SAMPLING_RATE_KHZ*1000);
    g_UsbCompositeDevPtr->audioUnified.recorderFreqControlRange.wMAX          = (g_UsbCompositeDevPtr->AUDIO_IN_SAMPLING_RATE_KHZ*1000);
    g_UsbCompositeDevPtr->audioUnified.recorderFreqControlRange.wRES          = 0U;

    g_UsbCompositeDevPtr->audioUnified.volumeControlRange.wNumSubRanges = 1U;
    g_UsbCompositeDevPtr->audioUnified.volumeControlRange.wMIN          = 0x8001U;
    g_UsbCompositeDevPtr->audioUnified.volumeControlRange.wMAX          = 0x7FFFU;
    g_UsbCompositeDevPtr->audioUnified.volumeControlRange.wRES          = 1U;

#endif
    g_UsbCompositeDevPtr->audioUnified.tdReadNumberPlay       = 0;
    g_UsbCompositeDevPtr->audioUnified.tdWriteNumberPlay      = 0;
    g_UsbCompositeDevPtr->audioUnified.tdWriteNumberRec       = 0;
    g_UsbCompositeDevPtr->audioUnified.tdReadNumberRec        = 0;
    g_UsbCompositeDevPtr->audioUnified.audioSendCount[0]      = 0;
    g_UsbCompositeDevPtr->audioUnified.audioSendCount[1]      = 0;
    g_UsbCompositeDevPtr->audioUnified.lastAudioSendCount     = 0;
    g_UsbCompositeDevPtr->audioUnified.usbRecvCount           = 0;
    g_UsbCompositeDevPtr->audioUnified.audioSendTimes         = 0;
    g_UsbCompositeDevPtr->audioUnified.usbRecvTimes           = 0;
    g_UsbCompositeDevPtr->audioUnified.audioRecvCount         = 0;
    g_UsbCompositeDevPtr->audioUnified.usbSendTimes           = 0;
    g_UsbCompositeDevPtr->audioUnified.startPlayFlag          = 0;
    g_UsbCompositeDevPtr->audioUnified.startPlayHalfFull      = 0;
    g_UsbCompositeDevPtr->audioUnified.startRec               = 0;
    g_UsbCompositeDevPtr->audioUnified.startRecHalfFull       = 0;
    g_UsbCompositeDevPtr->audioUnified.speakerIntervalCount   = 0;
    g_UsbCompositeDevPtr->audioUnified.speakerReservedSpace   = 0;
    g_UsbCompositeDevPtr->audioUnified.recorderReservedSpace  = 0;
    g_UsbCompositeDevPtr->audioUnified.timesFeedbackCalculate = 0;
    g_UsbCompositeDevPtr->audioUnified.speakerDetachOrNoInput = 0;
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
#if defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
    g_UsbCompositeDevPtr->audioUnified.froTrimIntervalCount     = 0;
    g_UsbCompositeDevPtr->audioUnified.usbFroTicksPrev          = 0;
    g_UsbCompositeDevPtr->audioUnified.usbFroTicksEma           = AUDIO_FRO_USB_SOF_INTERVAL_TICK_COUNT;
    g_UsbCompositeDevPtr->audioUnified.usbFroTickEmaFrac        = 0;
    g_UsbCompositeDevPtr->audioUnified.usbFroTickBasedPrecision = AUDIO_USB_FRO_TRIM_TICK_BASED_PRECISION;
#endif
    g_UsbCompositeDevPtr->audioUnified.curAudioPllFrac            = AUDIO_PLL_FRACTIONAL_DIVIDER;
    g_UsbCompositeDevPtr->audioUnified.audioPllTicksPrev          = 0;
    g_UsbCompositeDevPtr->audioUnified.audioPllTicksEma           = AUDIO_PLL_USB_SOF_INTERVAL_TICK_COUNT;
    g_UsbCompositeDevPtr->audioUnified.audioPllTickEmaFrac        = 0;
    g_UsbCompositeDevPtr->audioUnified.audioPllTickBasedPrecision = AUDIO_PLL_FRACTION_TICK_BASED_PRECISION;
    g_UsbCompositeDevPtr->audioUnified.stopDataLengthAudioAdjust  = 0U;
#endif
    return kStatus_USB_Success;
}

void USB_AudioCodecTask(void)
{
}

void USB_AudioSpeakerResetTask(void)
{
    if (g_UsbCompositeDevPtr->audioUnified.speakerDetachOrNoInput)
    {
        USB_DeviceAudioSpeakerStatusReset();
    }
}
