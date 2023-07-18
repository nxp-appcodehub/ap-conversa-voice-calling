/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _APP_GLOBAL_PLATFORM_H_
#define _APP_GLOBAL_PLATFORM_H_

#define APP_MIN(i, j) (((i) < (j)) ? (i) : (j))
#define APP_MAX(i, j) (((i) > (j)) ? (i) : (j))

#if  (APP_PLATFORM == APP_PL_RT1170EVK)

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*
 * Audio frame work define
 */
// RX path features & limitation
#define APP_AUDIO_RX_SAMPLE_PER_FRAME_MIN       16				   														   // minimum sample per frame for the RX path (unit is sample)
#define APP_AUDIO_RX_SAMPLE_PER_FRAME_MAX       128				   														   // maximum sample per frame for the RX path (unit is sample)
#define APP_AUDIO_RX_BIT_PER_SAMPLE_MAX  		32				   														   // maximum bit per sample for the RX path (unit is bit)
#define APP_AUDIO_RX_BYTES_PER_SAMPLE_MAX		APP_AUDIO_RX_BIT_PER_SAMPLE_MAX / 8;									   // maximum byte per sample for the RX path (unit is byte)
#define APP_AUDIO_RX_BYTE_PER_FRAME_MAX  	    (APP_AUDIO_RX_SAMPLE_PER_FRAME_MAX *APP_AUDIO_RX_BIT_PER_SAMPLE_MAX / 8)  // maximum number of byte per frame for the RX path (unit is frame)
#define APP_AUDIO_RX_SAMPLE_RATE_MAX            32000														               // maximum sample rate supported for the RX path (unit is Hertz)
#define APP_AUDIO_RX_CHANNEL_NUMBER_MAX			2																		   // maximum input channel of the RX path
#define APP_AUDIO_RX_REF_CHANNEL_NUMBER_MAX     1																		   // maximal number of Rx reference channel

// TX path features & limitation
#define APP_AUDIO_TX_SAMPLE_PER_FRAME_MIN       16				   														   // minimum sample per frame for the TX path (unit is sample)
#define APP_AUDIO_TX_SAMPLE_PER_FRAME_MAX       128				   														   // maximum sample per frame for the TX path (unit is sample)
#define APP_AUDIO_TX_BIT_PER_SAMPLE_MAX  		32				   														   // maximum bit per sample for the TX path (unit is bit)
#define APP_AUDIO_TX_BYTES_PER_SAMPLE_MAX		APP_AUDIO_TX_BIT_PER_SAMPLE_MAX / 8	   								       // maximum byte per sample for the TX path (unit is byte)
#define APP_AUDIO_TX_BYTE_PER_FRAME_MAX  	    (APP_AUDIO_TX_SAMPLE_PER_FRAME_MAX * APP_AUDIO_TX_BIT_PER_SAMPLE_MAX / 8)  // maximum number of byte per frame for the TX path (unit is frame)
#define APP_AUDIO_TX_SAMPLE_RATE_MAX            32000			  														   // maximum sample rate supported for the TX path (unit is Hertz)
#define APP_AUDIO_TX_CHANNEL_NUMBER_MAX			4										   								   // maximum input channel of the TX path

// PDM features & limitation
#define APP_AUDIO_PDM_BIT_PER_SAMPLE_MAX 		32									 // Maximal number of bit at PDM output SW interface
#define APP_AUDIO_PDM_CHANNEL_NUMBER_MAX		6		 							 // Maximum number of the PDM channel (used to decode the data provided by the PDM IP) 6 and not 4 because the 4 PDM mic are not PDM channel aligned
#define APP_AUDIO_PDM_MIC_SAMPLE_PER_FRAME_MAX 	APP_AUDIO_SAMPLE_PER_FRAME_MAX_TX	 // Maximum PDM sample rate supported (unit is sample)
#define APP_AUDIO_PDM_MIC_NUMBER_MAX 		    4   								 // Maximum number of PDM mic supported by the platform

// PDM initial configuration
#if (APP_AUDIO_PDM_MIC_NUMBER_MAX > 0)

	#define APP_AUDIO_PDM_BASE_ADDRESS			PDM								// PDM base address
	#define APP_AUDIO_PDM_CLK_FREQ          	24576000						// PDM clock value
	#define APP_AUDIO_PDM_FIFO_WATERMARK        4								// PDM FIFO threshold to provide a read request
	#define APP_AUDIO_PDM_QUALITY_MODE          kPDM_QualityModeHigh			// PDM quality mode
	#define APP_AUDIO_PDM_CIC_OVERSAMPLE_RATE   (0U)
	#define APP_AUDIO_PDM_ENABLE_CHANNEL_0   	(0U)							// PDM channel enabled
	#define APP_AUDIO_PDM_ENABLE_CHANNEL_1  	(1U)							// PDM channel enabled
	#define APP_AUDIO_PDM_ENABLE_CHANNEL_2   	(2U)							// PDM channel enabled
	#define APP_AUDIO_PDM_ENABLE_CHANNEL_3  	(3U)							// PDM channel enabled
	#define APP_AUDIO_PDM_ENABLE_CHANNEL_4   	(4U)							// PDM channel enabled
	#define APP_AUDIO_PDM_ENABLE_CHANNEL_5  	(5U)							// PDM channel enabled
	//#define APP_AUDIO_PDM_SAMPLE_CLOCK_RATE     6144000 						// PDM clock rate (unit is Hertz)
	#define APP_AUDIO_PDM_BIT_PER_SAMPLE		32 								// PDM bit per sample (unit is bit). This value is fixed due to hardware PDM and can't be changed
#endif

// EDMA initial configuration
#define APP_AUDIO_DMA_BASE_ADDRESS	      		DMA0				  			// DMA base address
#define APP_AUDIO_DMA_DMAMUX             		DMAMUX0				  			// DMAMUX0 base address
#define APP_AUDIO_EDMA_CHANNEL_SAI1TX    	    0  				  				// EDMA channel reserved for SAI0 Tx  (EDMA 0-16)
#define APP_AUDIO_EDMA_CHANNEL_PDM_MIC   	   	2					  			// EDMA channel reserved for PDM mic1 (EDMA 2-18)
#define APP_AUDIO_EDMA_REQUEST_SOURCE_SAI1TX  	kDmaRequestMuxSai1Tx  			// EDMA request source for SAI1 Tx
#define APP_AUDIO_EDMA_REQUEST_SOURCE_PDM_MIC 	kDmaRequestMuxPdm	  			// EDMA request source for PDM mic

// SAI
#define APP_AUDIO_SAI                	   SAI1
#define APP_AUDIO_SAI_CHANNEL              0
#define APP_AUDIO_SAI_CLK_FREQ             24576000

// AUDIO CODEC
// Volume range 79dB
// register value = (% * (127-48) + 48)
// register value 127 => + 6dB
// register value 121 => + 0dB
// register value  48 => -73dB
#define APP_AUDIO_CODEC_VOLUME_GAIN_6DB 		100U  // 100% => volume codec register = 127  => +6dB of gain on the codec output signal
#define APP_AUDIO_CODEC_VOLUME_GAIN_2DB 		 95U  //  95% => volume codec register = 123  => +2dB of gain on the codec output signal
#define APP_AUDIO_CODEC_VOLUME_ATTENUATION_2DB 	 90U  //  90% => volume codec register = 119  => -2dB of gain on the codec output signal

/*
 * USB
 */
// USB TX
#define APP_USB_TX_CHANNEL_MAX 					4																// Maximal number of channel in the USB Tx
#define APP_USB_TX_BYTES_MAX 					APP_AUDIO_TX_BYTES_PER_SAMPLE_MAX								// Maximal number of bytes per samples in the USB Tx
#define APP_USB_TX_FORMAT_BITS_MAX				APP_AUDIO_TX_BIT_PER_SAMPLE_MAX									// Maximal number of bits per samples in the USB Tx
#define APP_USB_TX_CIRCULAR_BUFFER_SIZE_BYTE 	(APP_AUDIO_TX_BYTE_PER_FRAME_MAX * APP_USB_TX_CHANNEL_MAX * 5)	// USB Tx circular buffer size (unit is byte)
#define APP_USB_TX_SAMPLING_RATE_KHZ_MAX   		(32000/1000)
// USB RX
#define APP_USB_RX_CHANNEL_MAX 					2																// Maximal number of channel in the USB Rx
#define APP_USB_RX_BYTES_MAX 					3																// Maximal number of bytes per samples in the USB Rx DO NOT MODIFIED
#define APP_USB_RX_FORMAT_BITS_MAX				24																// Maximal number of bits per samples in the USB Rx DO NOT MODIFIED
#define APP_USB_RX_FEEDBACK_SIZE_BYTE   		APP_USB_RX_CHANNEL_MAX * APP_USB_RX_BYTES_MAX   				// Maximal size allow for feedback calculation
#define APP_USB_RX_BYTE_PER_FRAME_MAX			APP_AUDIO_RX_SAMPLE_PER_FRAME_MAX * APP_USB_RX_BYTES_MAX
#define APP_USB_RX_CIRCULAR_BUFFER_SIZE_BYTE 	(APP_USB_RX_BYTE_PER_FRAME_MAX * APP_USB_RX_CHANNEL_MAX * 5)	// USB Rx circular buffer size (unit is byte)
#define APP_USB_RX_SAMPLING_RATE_KHZ_MAX   		(32000/1000)


// USB general
#define APP_USB_SAMPLING_RATE_16KHZ  16			// sampling rate value when sampling rate is 16Khz DO NOT MODIFIED

/* CHECK AUDIO FRAME WORK COMPATIBLITY */
#if APP_AUDIO_TX_CHANNEL_NUMBER_MAX > APP_AUDIO_PDM_MIC_NUMBER_MAX
	#error "ERROR: APP_AUDIO_TX_CHANNEL_NUMBER_MAX > APP_AUDIO_PDM_MIC_NUMBER_MAX"
#endif
#if APP_AUDIO_RX_CHANNEL_NUMBER_MAX < 2
	#error "ERROR: APP_AUDIO_RX_CHANNEL_NUMBER_MAX can't be < 2" // Audio codec is I2S so stereo is required
#endif

// Check USB RX circular buffer size is a multiple of USB RX frame size whatever the configuration
// 16K/16bits => 2 bytes per sample in USB RX circular buffer and USB RX is 16 samples per frame (1ms)
// 24K/16bits => 2 bytes per sample in USB RX circular buffer and USB RX is 24 samples per frame (1ms)
// 32K/16bits => 2 bytes per sample in USB RX circular buffer and USB RX is 32 samples per frame (1ms)
#if APP_USB_RX_CIRCULAR_BUFFER_SIZE_BYTE % 2 != 0
	#error "ERROR: USB Rx configuration with 2 Bytes per sample is not a multiple of APP_USB_RX_CIRCULAR_BUFFER_SIZE_BYTE"
#endif
// 16K/32bits => 3 bytes per sample in USB RX circular buffer and USB RX is 16 samples per frame (1ms)
// 24K/32bits => 3 bytes per sample in USB RX circular buffer and USB RX is 24 samples per frame (1ms)
// 32K/32bits => 3 bytes per sample in USB RX circular buffer and USB RX is 32 samples per frame (1ms)
#if APP_USB_RX_CIRCULAR_BUFFER_SIZE_BYTE % 3 != 0
	#error "ERROR: USB Rx configuration with 3 Bytes per sample is not a multiple of APP_USB_RX_CIRCULAR_BUFFER_SIZE_BYTE"
#endif

// Priority
#define IRQ_PRIORITY_RX_SAI_DMA 5U   // Interrupt priority for DMA SAI RX
#define IRQ_PRIORITY_TX_PDM_DMA 5U	 // Interrupt priority for DMA PDM TX
#define IRQ_PRIORITY_USB        8U   // USB interrupt priority
/*******************************************************************************
 * Enums
 ******************************************************************************/
// platform And Materials
typedef enum
{
    APP_PLATFORM_MATERIALS_RT1170,			 // RT1170 only
	APP_PLATFORM_MATERIALS_RT1170_TB2136,    // RT1170 + amplifier wondom TA024 + 1xTB2136 speaker
	APP_PLATFORM_MATERIALS_RT1170_SHB_AMP2,  // RT1170 + amplifier wondom TA024 + Steelhead box microphone and speaker
}APP_platformMaterials_en;


#endif /* APP_PLATFORM == */

#endif /* _APP_GLOBAL_PLATFORM_H_ */
