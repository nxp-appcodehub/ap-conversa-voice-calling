/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __USB_AUDIO_H__
#define __USB_AUDIO_H__ 1

#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"
#include "usb_device_class.h"
#include "usb_device_audio.h"
#include "usb_device_cdc_acm.h"
#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"
#include "virtual_com.h"
#include "tools.h"
#if ((defined FSL_FEATURE_SOC_USBPHY_COUNT) && (FSL_FEATURE_SOC_USBPHY_COUNT > 0U))
	#include "usb_phy.h"
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/



#define USB_AUDIO_ENTER_CRITICAL() \
                                   \
    OSA_SR_ALLOC();                \
                                   \
    OSA_ENTER_CRITICAL()

#define USB_AUDIO_EXIT_CRITICAL() OSA_EXIT_CRITICAL()


/*audio data buffer depth*/
#define AUDIO_BUFFER_UPPER_LIMIT(x) (((x)*5) / 8)
#define AUDIO_BUFFER_LOWER_LIMIT(x) (((x)*3) / 8)

#define TSAMFREQ2BYTES(f)     (f & 0xFFU), ((f >> 8U) & 0xFFU), ((f >> 16U) & 0xFFU)
#define TSAMFREQ2BYTESHS(f)   (f & 0xFFU), ((f >> 8U) & 0xFFU), ((f >> 16U) & 0xFFU), ((f >> 24U) & 0xFFU)
#define AUDIO_ADJUST_MIN_STEP (0x01)


#define AUDIO_FRO_USB_SOF_INTERVAL_VALID_DEVIATION ((AUDIO_FRO_USB_SOF_INTERVAL_TICK_COUNT) >> 7)
#define AUDIO_PLL_USB_SOF_INTERVAL_VALID_DEVIATION ((AUDIO_PLL_USB_SOF_INTERVAL_TICK_COUNT) >> 7)

#define MUTE_CODEC_TASK    (1UL << 0U)
#define UNMUTE_CODEC_TASK  (1UL << 1U)
#define VOLUME_CHANGE_TASK (1UL << 2U)

typedef struct _usb_audio_composite_struct
{
    usb_device_handle deviceHandle;    /* USB device handle.                   */
    class_handle_t audioSpeakerHandle; /* USB AUDIO GENERATOR class handle.    */
    PL_UINT32 currentStreamOutMaxPacketSize;
    PL_UINT32 currentFeedbackMaxPacketSize;
    class_handle_t audioRecorderHandle;
    uint8_t copyProtect;
    uint8_t curSpeakerMute;
    uint8_t curMicrophoneMute;
    uint8_t curSpeakerVolume[2];
    uint8_t curMicrophoneVolume[2];
    uint8_t minVolume[2];
    uint8_t maxVolume[2];
    uint8_t resVolume[2];
    uint8_t curBass;
    uint8_t minBass;
    uint8_t maxBass;
    uint8_t resBass;
    uint8_t curMid;
    uint8_t minMid;
    uint8_t maxMid;
    uint8_t resMid;
    uint8_t curTreble;
    uint8_t minTreble;
    uint8_t maxTreble;
    uint8_t resTreble;
    uint8_t curAutomaticGain;
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
    uint8_t curDelay[4];
    uint8_t minDelay[4];
    uint8_t maxDelay[4];
    uint8_t resDelay[4];
#else
    uint8_t curDelay[2];
    uint8_t minDelay[2];
    uint8_t maxDelay[2];
    uint8_t resDelay[2];
#endif
    uint8_t curLoudness;
    uint8_t curSamplingFrequency[3];
    uint8_t minSamplingFrequency[3];
    uint8_t maxSamplingFrequency[3];
    uint8_t resSamplingFrequency[3];
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
    uint8_t curSpeakerMute20;
    uint8_t curMicrophoneMute20;
    uint8_t curClockValid;
    uint8_t curSpeakerVolume20[2];
    uint8_t curMicrophoneVolume20[2];
    PL_UINT32 curSpeakerSampleFrequency;
    PL_UINT32 curRecorderSampleFrequency;
    usb_device_control_range_layout3_struct_t speakerFreqControlRange;
    usb_device_control_range_layout3_struct_t recorderFreqControlRange;
    usb_device_control_range_layout2_struct_t volumeControlRange;
#endif
    uint8_t currentConfiguration;
    uint8_t currentInterfaceAlternateSetting[USB_AUDIO_COMPOSITE_INTERFACE_COUNT];
    uint8_t speed;
    uint8_t attach;
    volatile uint8_t startPlayFlag;
    volatile uint8_t startPlayHalfFull;
    volatile uint8_t startRec;
    volatile uint8_t startRecHalfFull;
    volatile PL_UINT32 tdReadNumberPlay;
    volatile PL_UINT32 tdWriteNumberPlay;
    volatile PL_UINT32 audioSpeakerReadDataCount[2];
    volatile PL_UINT32 audioSpeakerWriteDataCount[2];
    volatile PL_UINT32 tdWriteNumberRec;
    volatile PL_UINT32 tdReadNumberRec;
    volatile PL_UINT32 audioSendCount[2];
    volatile PL_UINT32 lastAudioSendCount;
    volatile PL_UINT32 usbRecvCount;
    volatile PL_UINT32 audioSendTimes;
    volatile PL_UINT32 usbRecvTimes;
    volatile PL_UINT32 audioRecvCount;
    volatile PL_UINT32 usbSendTimes;
    volatile PL_UINT32 speakerIntervalCount;
    volatile PL_UINT32 speakerReservedSpace;
    volatile PL_UINT32 recorderReservedSpace;
    volatile PL_UINT32 timesFeedbackCalculate;
    volatile PL_UINT32 speakerDetachOrNoInput;
    volatile PL_UINT32 codecSpeakerTask;
    volatile PL_UINT32 codecMicrophoneTask;
    PL_UINT32 audioPlayTransferSize;
    volatile uint16_t audioPlayBufferSize;
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
    volatile PL_UINT32 curAudioPllFrac;
    volatile PL_UINT32 audioPllTicksPrev;
    volatile int32_t audioPllTicksDiff;
    volatile int32_t audioPllTicksEma;
    volatile int32_t audioPllTickEmaFrac;
    volatile int32_t audioPllTickBasedPrecision;
    volatile uint8_t stopDataLengthAudioAdjust;
#if defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
    volatile PL_UINT32 froTrimIntervalCount;
    volatile PL_UINT32 usbFroTicksPrev;
    volatile int32_t usbFroTicksEma;
    volatile int32_t usbFroTickEmaFrac;
    volatile int32_t usbFroTickBasedPrecision;
#endif
#else
    volatile PL_UINT32 maxFrameCount;
    volatile PL_UINT32 lastFrameCount;
    volatile PL_UINT32 currentFrameCount;
    volatile uint8_t firstCalculateFeedback;
    volatile uint8_t stopFeedbackUpdate;
    volatile PL_UINT32 lastFeedbackValue;
    volatile uint8_t feedbackDiscardFlag;
    volatile uint8_t feedbackDiscardTimes;
#endif
} usb_audio_composite_struct_t;

typedef struct _usb_device_composite_struct
{
    usb_device_handle deviceHandle;
    usb_audio_composite_struct_t audioUnified;
    usb_cdc_vcom_struct_t cdcVcom;  /* CDC virtual com device structure. */
    uint8_t speed;
    uint8_t attach;
    uint8_t currentConfiguration;
    uint8_t currentInterfaceAlternateSetting[USB_DEVICE_INTERFACE_COUNT];


    //-----the following repleaces the orignal macro definitions

    uint8_t AUDIO_OUT_FORMAT_CHANNELS;											// number of usb RX channel number			2,3,4
    uint8_t AUDIO_IN_FORMAT_CHANNELS;											// number of usb TX channel number			1,2
    uint8_t AUDIO_IN_FORMAT_SIZE;												// number of bits per sample in USBRX flow    2 or 4 (bytes)
    uint8_t AUDIO_OUT_FORMAT_SIZE;												// number of bits per sample in USBRX flow    2 or 4 (bytes)

    uint8_t AUDIO_IN_SAMPLING_RATE_KHZ;
	uint8_t AUDIO_OUT_SAMPLING_RATE_KHZ;
	uint16_t AUDIO_OUT_TRANSFER_LENGTH_ONE_FRAME;				//#define AUDIO_OUT_TRANSFER_LENGTH_ONE_FRAME (AUDIO_OUT_SAMPLING_RATE_KHZ * AUDIO_OUT_FORMAT_CHANNELS * AUDIO_OUT_FORMAT_SIZE)

	uint16_t FS_ISO_OUT_ENDP_PACKET_SIZE; 						//#define FS_ISO_OUT_ENDP_PACKET_SIZE (AUDIO_OUT_TRANSFER_LENGTH_ONE_FRAME)
	uint16_t FS_ISO_IN_ENDP_PACKET_SIZE; 						//#define FS_ISO_IN_ENDP_PACKET_SIZE  (AUDIO_IN_SAMPLING_RATE_KHZ * AUDIO_IN_FORMAT_CHANNELS * AUDIO_IN_FORMAT_SIZE)
	uint16_t HS_ISO_OUT_ENDP_PACKET_SIZE; 						//#define HS_ISO_OUT_ENDP_PACKET_SIZE (AUDIO_OUT_TRANSFER_LENGTH_ONE_FRAME)
	uint16_t HS_ISO_IN_ENDP_PACKET_SIZE; 						//#define HS_ISO_IN_ENDP_PACKET_SIZE  (AUDIO_IN_SAMPLING_RATE_KHZ * AUDIO_IN_FORMAT_CHANNELS * AUDIO_IN_FORMAT_SIZE)
	TOOLS_circularBuffer_st cirBuffTx;									  /* Circular buffer structure for TX Path*/
	TOOLS_circularBuffer_st cirBuffRx;									  /* Circular buffer structure for RX path*/


} usb_device_composite_struct_t;



#endif /* __USB_AUDIO_GENERATOR_H__ */
