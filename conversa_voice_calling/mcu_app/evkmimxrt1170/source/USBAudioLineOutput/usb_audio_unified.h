/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016,2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __USB_AUDIO_H__
#define __USB_AUDIO_H__


#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"
#include "usb_device_class.h"
#include "usb_device_audio.h"
#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"
#include "usb_virtual_com.h"

// Circular Buffer
#include "tools.h"

#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
#include "fsl_sysmpu.h"
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */
#if ((defined FSL_FEATURE_SOC_USBPHY_COUNT) && (FSL_FEATURE_SOC_USBPHY_COUNT > 0U))
#include "usb_phy.h"
#endif

/*******************************************************************************
* Definitions
******************************************************************************/
#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)
#define CONTROLLER_ID kUSB_ControllerEhci0
#endif
#if defined(USB_DEVICE_CONFIG_KHCI) && (USB_DEVICE_CONFIG_KHCI > 0U)
#define CONTROLLER_ID kUSB_ControllerKhci0
#endif
#if defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
#define CONTROLLER_ID kUSB_ControllerLpcIp3511Fs0
#endif
#if defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U)
#define CONTROLLER_ID kUSB_ControllerLpcIp3511Hs0
#endif



#define AUDIO_SAMPLING_RATE_TO_10_14 (APP_USB_RX_SAMPLING_RATE_KHZ_MAX << 10)
#if defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U)
	#define AUDIO_SAMPLING_RATE_TO_16_16_SPECIFIC \
		((APP_USB_RX_SAMPLING_RATE_KHZ_MAX / (AUDIO_OUT_TRANSFER_LENGTH_ONE_FRAME / HS_ISO_OUT_ENDP_PACKET_SIZE)) << 12)
#elif defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)
//already commented	//#define AUDIO_SAMPLING_RATE_TO_16_16 \
//already commented	//	((APP_USB_RX_SAMPLING_RATE_KHZ_MAX / (AUDIO_OUT_TRANSFER_LENGTH_ONE_FRAME / HS_ISO_OUT_ENDP_PACKET_SIZE)) << 13)
#endif

#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
#else
	extern volatile uint8_t feedbackValueUpdating;
#endif
#define USB_AUDIO_CONTROL_INTERFACE_ALTERNATE_COUNT         (1)
#define USB_AUDIO_RECORDER_STREAM_INTERFACE_ALTERNATE_COUNT (2)
#define USB_AUDIO_SPEAKER_STREAM_INTERFACE_ALTERNATE_COUNT  (2)
#define USB_AUDIO_CONTROL_INTERFACE_ALTERNATE_0             (0)
#define USB_AUDIO_RECORDER_STREAM_INTERFACE_ALTERNATE_0     (0)
#define USB_AUDIO_RECORDER_STREAM_INTERFACE_ALTERNATE_1     (1)
#define USB_AUDIO_SPEAKER_STREAM_INTERFACE_ALTERNATE_0      (0)
#define USB_AUDIO_SPEAKER_STREAM_INTERFACE_ALTERNATE_1      (1)

#define AUDIO_SAMPLING_RATE_TO_10_14 (APP_USB_RX_SAMPLING_RATE_KHZ_MAX << 10)
#define AUDIO_SAMPLING_RATE_TO_16_16 (APP_USB_RX_SAMPLING_RATE_KHZ_MAX << 13)
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
	#if defined(USB_DEVICE_WORKAROUND_AUDIO_20_WINDOWS) && (USB_DEVICE_WORKAROUND_AUDIO_20_WINDOWS > 0)
		/* change 10.10 data to 10.14 or 16.16 (12.13) */
		#define AUDIO_UPDATE_FEEDBACK_DATA(m, n)                  \
			{                                                     \
				m[0] = (((n & 0x000003FFu) << 3) & 0xFFu);        \
				m[1] = ((((n & 0x000003FFu) << 3) >> 8) & 0xFFu); \
				m[2] = (((n & 0x000FFC00u) >> 10) & 0xFFu);       \
				m[3] = (((n & 0x000FFC00u) >> 18) & 0xFFu);       \
			}
	#else
		/* change 10.10 data to 10.14 or 16.16 (12.13) */
		#define AUDIO_UPDATE_FEEDBACK_DATA(m, n)                  \
			if (USB_SPEED_HIGH == g_appHandle.usbTxRx_handle.audioUnified.speed)              \
			{                                                     \
				m[0] = (((n & 0x000003FFu) << 3) & 0xFFu);        \
				m[1] = ((((n & 0x000003FFu) << 3) >> 8) & 0xFFu); \
				m[2] = (((n & 0x000FFC00u) >> 10) & 0xFFu);       \
				m[3] = (((n & 0x000FFC00u) >> 18) & 0xFFu);       \
			}                                                     \
			else                                                  \
			{                                                     \
				m[0] = ((n << 4) & 0xFFU);                        \
				m[1] = (((n << 4) >> 8U) & 0xFFU);                \
				m[2] = (((n << 4) >> 16U) & 0xFFU);               \
			}
	#endif
#else
	/* change 10.10 data to 10.14 */
	#define AUDIO_UPDATE_FEEDBACK_DATA(m, n)                     \
		{                                                        \
			feedbackValueUpdating = 1;                           \
			m[0]                  = ((n << 4) & 0xFFU);          \
			m[1]                  = (((n << 4) >> 8U) & 0xFFU);  \
			m[2]                  = (((n << 4) >> 16U) & 0xFFU); \
			feedbackValueUpdating = 0;                           \
		}
#endif

#define AUDIO_SPEAKER_DATA_WHOLE_BUFFER_LENGTH  (12*2)		//was 32*2 --- to fit both frame size=48 and 72, should be a value can be divided by both 2 and 3, so: 12*2 is OK, 6*2 is OK, but 8*2, 16*2 is not OK to use here
															//set to (6*2) is too small, will have noise



#define AUDIO_BUFFER_UPPER_LIMIT(x)             (((x)*5) / 8)
#define AUDIO_BUFFER_LOWER_LIMIT(x)             (((x)*3) / 8)
#if defined(USB_DEVICE_CONFIG_AUDIO_CLASS_2_0) && (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0 > 0U)
	#define AUDIO_CALCULATE_Ff_INTERVAL (16) /* suggest: 1024U, 512U, 256U */
#else
	/* feedback calculate interval */
	#define AUDIO_CALCULATE_Ff_INTERVAL (16U)
	/* the threshold transfer count that can tolerance by frame */
	#define USB_AUDIO_PLAY_BUFFER_FEEDBACK_TOLERANCE_THRESHOLD (4U)
	/* feedback value discard times, the first feedback vaules are discarded */
	#define AUDIO_SPEAKER_FEEDBACK_DISCARD_COUNT (4U)
#endif
#define TSAMFREQ2BYTES(f)                       (f & 0xFFU), ((f >> 8U) & 0xFFU), ((f >> 16U) & 0xFFU)
#define TSAMFREQ2BYTESHS(f)                     (f & 0xFFU), ((f >> 8U) & 0xFFU), ((f >> 16U) & 0xFFU), ((f >> 24U) & 0xFFU)
#define AUDIO_ADJUST_MIN_STEP                   (0x01)

#define AUDIO_OUT_TRANSFER_LENGTH_ONE_FRAME 	(APP_USB_RX_SAMPLING_RATE_KHZ_MAX * APP_USB_RX_CHANNEL_MAX * APP_USB_RX_BYTES_MAX)
#define AUDIO_PLAY_BUFFER_SIZE_ONE_FRAME 		AUDIO_OUT_TRANSFER_LENGTH_ONE_FRAME

#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
/**********************************************************************
Audio PLL contants
      AUDIO_PLL_USB1_SOF_INTERVAL_COUNT
      The Audio PLL clock is 24.576Mhz, and the USB1_SOF_TOGGLE frequency is 4kHz when the device is attached,
      so AUDIO_PLL_USB1_SOF_INTERVAL_COUNT = (24576000 * 100 (stands for counter interval)) /4000 = 614400
      AUDIO_PLL_FRACTIONAL_CHANGE_STEP
      The Audio input clock is 24Mhz, and denominator is 4500, divider is 15 and PFD is 26.
      so AUDIO_PLL_FRACTIONAL_CHANGE_STEP = (24000000 * 100 (stands for counter interval) * 18) / (27000 * 26 * 15
*4000) + 1
**********************************************************************/
#define AUDIO_PLL_USB1_SOF_INTERVAL_COUNT  (614400) /* The USB1_SOF_TOGGLE's frequency is 4kHz. */
#define AUDIO_PLL_USB1_SOF_INTERVAL_COUNT1 (491520) /* The USB1_SOF_TOGGLE's frequency is 4kHz. */
#define AUDIO_PLL_FRACTIONAL_CHANGE_STEP   (2)
#endif

#define MUTE_CODEC_TASK    (1UL << 0U)
#define UNMUTE_CODEC_TASK  (1UL << 1U)
#define VOLUME_CHANGE_TASK (1UL << 2U)
/*******************************************************************************
*  STUCTURE
******************************************************************************/
typedef struct _usb_audio_composite_struct
{
    usb_device_handle deviceHandle;    /* USB device handle.                   */
    class_handle_t audioSpeakerHandle; /* USB AUDIO GENERATOR class handle.    */
    uint32_t currentStreamOutMaxPacketSize;
    uint32_t currentFeedbackMaxPacketSize;
    class_handle_t audioRecorderHandle;
    uint8_t copyProtect;
    uint8_t curMute;
    uint8_t curVolume[2];
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
    uint8_t curDelay[2];
    uint8_t minDelay[2];
    uint8_t maxDelay[2];
    uint8_t resDelay[2];
    uint8_t curLoudness;
    uint8_t curSamplingFrequency[3];
    uint8_t minSamplingFrequency[3];
    uint8_t maxSamplingFrequency[3];
    uint8_t resSamplingFrequency[3];
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
    uint8_t curMute20;
    uint8_t curClockValid;
    uint8_t curVolume20[2];
    uint32_t curSampleFrequency;
    usb_device_control_range_layout3_struct_t freqControlRange;
    usb_device_control_range_layout2_struct_t volumeControlRange;
#endif
    uint8_t currentConfiguration;
    uint8_t currentInterfaceAlternateSetting[USB_AUDIO_COMPOSITE_INTERFACE_COUNT];
    uint8_t speed;
    uint8_t attach;
    uint8_t startRecording;
    volatile uint8_t startPlay;
    volatile uint32_t audioSendCount;
    volatile uint32_t lastAudioSendCount;
    volatile uint32_t usbRecvCount;
    volatile uint32_t speakerIntervalCount;
    volatile uint32_t UsbDnStrmBuf_OccupiedSpaceInByte;
    volatile uint32_t timesFeedbackCalculate;
    volatile uint32_t speakerDetachOrNoInput;
    volatile uint32_t codecTask;
    uint32_t audioPlayTransferSize;
    volatile uint16_t audioPlayBufferSize;

#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
    volatile uint32_t curAudioPllFrac;
    volatile uint32_t audioPllTicksPrev;
    volatile int32_t audioPllTicksDiff;
    volatile int32_t audioPllTicksEma;
    volatile int32_t audioPllTickEmaFrac;
    volatile int32_t audioPllStep;
#else
    volatile uint32_t maxFrameCount;
    volatile uint32_t lastFrameCount;
    volatile uint32_t currentFrameCount;
    volatile uint8_t firstCalculateFeedback;
    volatile uint8_t stopFeedbackUpdate;
    volatile uint32_t lastUplinkFeedbackValue;
    volatile uint32_t lastUsedSpace;
    volatile uint8_t feedbackDiscardFlag;
    volatile uint8_t feedbackDiscardTimes;
#endif
    uint32_t iso_in_endp_packet_size;
    uint32_t iso_out_endp_packet_size;
} usb_audio_composite_struct_t;

/* COMPOSITE STRUCTURE */
typedef struct _usb_device_composite_struct
{
    usb_device_handle 			 deviceHandle; 							  /* USB device handle. */
	usb_audio_composite_struct_t audioUnified;
    usb_cdc_vcom_struct_t 		 cdcVcom;  								  /* CDC virtual com device structure. */

    uint8_t speed;            											  /* Speed of USB device. USB_SPEED_FULL/USB_SPEED_LOW/USB_SPEED_HIGH.                 */
    uint8_t attach;           											  /* A flag to indicate whether a usb device is attached. 1: attached, 0: not attached */
    uint8_t currentConfiguration; 										  /* Current configuration value. */
    uint8_t currentInterfaceAlternateSetting[USB_DEVICE_INTERFACE_COUNT]; /* Current alternate setting value for each interface. */
    TOOLS_circularBuffer_st cirBuffTx;									  /* Circular buffer structure for TX Path*/
    TOOLS_circularBuffer_st cirBuffRx;									  /* Circular buffer structure for RX path*/
    uint8_t USBTxchannelNumber;											  // number of usb TX channel number
    uint8_t USBRxchannelNumber;											  // number of usb RX channel number
    uint8_t USBRxBitDepth;												  // number of bits per sample in USBRX flow
} usb_device_composite_struct_t;

/**********************************/
/* VARIABLES */

extern usb_phy_config_struct_t 				 g_phyConfig;
extern usb_device_class_config_list_struct_t g_UsbDeviceCompositeConfigList;
extern uint8_t 								 UsbAudioUpStreamingIsStarted;
extern uint8_t 								 UsbAudioDnStreamingIsStarted;
extern uint8_t audioPlayDataBuff[AUDIO_SPEAKER_DATA_WHOLE_BUFFER_LENGTH * FS_ISO_OUT_ENDP_PACKET_SIZE_MAX]; // USB buffer circular (global)
extern usb_device_composite_struct_t g_composite;



/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            USB_DeviceClockInit										             	 */
/*                                                                                               */
/* DESCRIPTION:    init usb Clock                                  								 */
/* PARAMETERS:                                                                                   */
/*************************************************************************************************/
void USB_DeviceClockInit(void);

/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            USB_DeviceIsrEnable										             	 */
/*                                                                                               */
/* DESCRIPTION:    enable USB interrupts                                  						 */
/* PARAMETERS:                                                                                   */
/*************************************************************************************************/
void USB_DeviceIsrEnable(void);


/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            USB_Init										             	 		 */
/*                                                                                               */
/* DESCRIPTION:    init USB composite device with Speaker, Recorder and Virtual com interfaces   */
/* PARAMETERS:                                                                                   */
/*					usbCompositeStruct   composite struct 										 */
/*************************************************************************************************/
status_t USB_Init(usb_device_composite_struct_t *p_usbCompositeStruct);

/*!
 * @brief Audio class specific callback function.
 *
 * This function handles the Audio class specific requests.
 *
 * @param handle		  The Audio class handle.
 * @param event 		  The Audio class event type.
 * @param param 		  The parameter of the class specific request.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
extern usb_status_t USB_DeviceAudioCompositeCallback(class_handle_t handle, uint32_t event, void *param);
/*!
 * @brief Audio device set configuration function.
 *
 * This function sets configuration for Audio class.
 *
 * @param handle The Audio class handle.
 * @param configure The Audio class configure index.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
extern usb_status_t USB_DeviceAudioCompositeSetConfigure(class_handle_t handle, uint8_t configure);
/*!
 * @brief Audio device initialization function.
 *
 * This function initializes the device with the composite device class information.
 *
 * @param device_composite The pointer to the composite device structure.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
extern usb_status_t USB_DeviceAudioCompositeInit(usb_device_composite_struct_t *device_composite);
extern usb_status_t USB_DeviceAudioRecorderSetInterface(class_handle_t handle,
                                                        uint8_t interface,
                                                        uint8_t alternateSetting);
extern usb_status_t USB_DeviceAudioSpeakerSetInterface(class_handle_t handle,
                                                       uint8_t interface,
                                                       uint8_t alternateSetting);
void USB_AudioCodecTask(void);
usb_status_t USB_DownlinkFeedbackUpdate();
PL_UINT32 USB_UplinkFeedbackPacketLengthAdjust();
usb_status_t USB_DeviceAudioRequest(class_handle_t handle, uint32_t event, void *param);
usb_status_t USB_DeviceCallback(usb_device_handle handle, uint32_t event, void *param);
void USB_DeviceAudioSpeakerStatusReset(void);

#endif /* __USB_AUDIO_GENERATOR_H__ */
