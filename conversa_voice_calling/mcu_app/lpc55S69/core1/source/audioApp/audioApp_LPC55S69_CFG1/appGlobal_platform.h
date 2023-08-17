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

#if  (APP_PLATFORM == APP_PL_LPC55S69EVK)
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*
 *   Define GPIO text to print according platform selection
 */
#ifdef MIPS_MEASURE_GPIO
#include "fsl_gpio.h"
#define     BOARD_USER_MIPS_GPIO_0_PIN                      5
#define     BOARD_USER_MIPS_GPIO_1_PIN                      8
#define     BOARD_USER_MIPS_GPIO_3_PIN                      10
#define     BOARD_USER_MIPS_GPIO_4_PIN                      11
#define     GPIO_0_Up()                   					GPIO_PinWrite(GPIO, 1, BOARD_USER_MIPS_GPIO_0_PIN, 1)
#define     GPIO_0_Dn()                   					GPIO_PinWrite(GPIO, 1, BOARD_USER_MIPS_GPIO_0_PIN, 0)
#define     GPIO_1_Up()                   					GPIO_PinWrite(GPIO, 1, BOARD_USER_MIPS_GPIO_1_PIN, 1)
#define     GPIO_1_Dn()                   					GPIO_PinWrite(GPIO, 1, BOARD_USER_MIPS_GPIO_1_PIN, 0)
#define     GPIO_3_Up()                   					GPIO_PinWrite(GPIO, 1, BOARD_USER_MIPS_GPIO_3_PIN, 1)
#define     GPIO_3_Dn()                   					GPIO_PinWrite(GPIO, 1, BOARD_USER_MIPS_GPIO_3_PIN, 0)
#define     GPIO_4_Up()                   					GPIO_PinWrite(GPIO, 1, BOARD_USER_MIPS_GPIO_4_PIN, 1)
#define     GPIO_4_Dn()                   					GPIO_PinWrite(GPIO, 1, BOARD_USER_MIPS_GPIO_4_PIN, 0)
#define		APP_GPIO_0_PURPOSE_TEXT							"GPIO_0 / P17 Pin 17 / CORE1:Time spent in audioRxTxProcess "
#define 	APP_GPIO_1_PURPOSE_TEXT							"GPIO_1 / P17 Pin 19 / CORE0:Time of swProcessConversa()"
#define 	APP_GPIO_3_PURPOSE_TEXT							"GPIO_3 / P18 Pin  3 / CORE1:Time spent in EDMA I2S ISR (I2S RX & I2S TX on the same pin)"
#define 	APP_GPIO_4_PURPOSE_TEXT							"GPIO_4 / P17 Pin  6 / CORE1:Ping/Pong Status of Core1 Process (UP:process use Ping/ DOWN: process use Pong )"
#endif

/*
 * DMA
 */
#define APP_AUDIO_DMA_BASE_ADDRESS 				(DMA0)
#define APP_AUDIO_DMA_NUMBER_OF_LINKED_DESC		2
/*
 * I2S
 */
#define APP_AUDIO_I2S_CLK_FREQ  				(24576000)
#define APP_AUDIO_I2S_RX_SAMPLERATE				16000
#define APP_AUDIO_I2S_TX_SAMPLERATE				32000
#define APP_AUDIO_I2S_BIT_PER_SAMPLE			32
#define APP_AUDIO_I2S_RX_CLOCK_DIVIDER          (APP_AUDIO_I2S_CLK_FREQ / APP_AUDIO_I2S_RX_SAMPLERATE / APP_AUDIO_I2S_BIT_PER_SAMPLE / 2U)		//core1 doesn't really confiugre PLL0, so use 24.576M directly
#define APP_AUDIO_I2S_TX_CLOCK_DIVIDER          (APP_AUDIO_I2S_CLK_FREQ / APP_AUDIO_I2S_TX_SAMPLERATE / APP_AUDIO_I2S_BIT_PER_SAMPLE / 2U)		//core1 doesn't really confiugre PLL0, so use 24.576M directly
/*
 * Audio frame work define
 */

//RX
#define APP_AUDIO_I2S_RX_MODE 					(kI2S_MasterSlaveNormalMaster)
#define APP_AUDIO_I2S_RX_SINK                   (I2S7)
#define APP_AUDIO_I2S_RX_SINK_CHANNEL  			(19)		//flexcomm 7
#define APP_AUDIO_I2S_RX_BIT_PER_SAMPLE			32
#define APP_AUDIO_I2S_RX_BIT_PER_FRAME			64		    // one I2S frame is 2 samples long ( Spl Left, spl Right)

//TX
#define APP_AUDIO_I2S_TX_MODE 					kI2S_MasterSlaveNormalMaster//kI2S_MasterSlaveNormalSlave//(kI2S_MasterSlaveNormalMaster)
#define APP_AUDIO_I2S_TX_SOURCE0 				(I2S6)
#define APP_AUDIO_I2S_TX_SOURCE0_CHANNEL 		(16)		//flexcomm 6
#define APP_AUDIO_I2S_TX_SOURCE1                (I2S2)
#define APP_AUDIO_I2S_TX_SOURCE1_CHANNEL 		(10)		//flexcomm 2
#define APP_AUDIO_I2S_TX_SOURCE2 				(I2S1)
#define APP_AUDIO_I2S_TX_SOURCE2_CHANNEL 		(6)		    //flexcomm 1
#define APP_AUDIO_I2S_TX_SOURCE3 				(I2S5)
#define APP_AUDIO_I2S_TX_SOURCE3_CHANNEL 		(14)		//flexcomm 5
#define APP_AUDIO_I2S_TX_BIT_PER_SAMPLE			32
#define APP_AUDIO_I2S_TX_BIT_PER_FRAME			64		    // one I2S frame is 2 samples long ( Spl Left, spl Right)
#define APP_AUDIO_I2S_TX_MAX_MICS_PER_CH		2		    //one I2S channel can have max 2 mics
#define RDSP_MICARRAY_MASK						0xFFFFF000

// RX path features & limitation
#define APP_AUDIO_RX_SAMPLE_PER_FRAME_MIN       16				   														   // minimum sample per frame for the RX path (unit is sample)
#define APP_AUDIO_RX_SAMPLE_PER_FRAME_MAX       120			   														   // maximum sample per frame for the RX path (unit is sample)
#define APP_AUDIO_RX_BIT_PER_SAMPLE_MAX  		32				   														   // maximum bit per sample for the RX path (unit is bit)
#define APP_AUDIO_RX_BYTES_PER_SAMPLE_MAX		APP_AUDIO_RX_BIT_PER_SAMPLE_MAX / 8									   // maximum byte per sample for the RX path (unit is byte)
#define APP_AUDIO_RX_BYTE_PER_FRAME_MAX  	    (APP_AUDIO_RX_SAMPLE_PER_FRAME_MAX *APP_AUDIO_RX_BIT_PER_SAMPLE_MAX / 8)  // maximum number of byte per frame for the RX path (unit is frame)
#define APP_AUDIO_RX_SAMPLE_RATE_MAX            16000 														               // maximum sample rate supported for the RX path (unit is Hertz)
#define APP_AUDIO_RX_CHANNEL_NUMBER_MAX			2																		   // maximum input channel of the RX path
#define APP_AUDIO_RX_REF_CHANNEL_NUMBER_MAX     1																		   // maximal number of Rx reference channel

// TX path features & limitation
#define APP_AUDIO_TX_SAMPLE_PER_FRAME_MIN       16				   														   // minimum sample per frame for the TX path (unit is sample)
#define APP_AUDIO_TX_SAMPLE_PER_FRAME_MAX       120			   														   	   // maximum sample per frame for the TX path (unit is sample)
#define APP_AUDIO_TX_SAMPLE_PER_FRAME_RDSPMIC   128			   														       // maximum sample per frame for the TX path (unit is sample)
#define APP_AUDIO_TX_BIT_PER_SAMPLE_MAX  		32				   														   // maximum bit per sample for the TX path (unit is bit)
#define APP_AUDIO_TX_BYTES_PER_SAMPLE_MAX		APP_AUDIO_TX_BIT_PER_SAMPLE_MAX / 8	   								       // maximum byte per sample for the TX path (unit is byte)
#define APP_AUDIO_TX_BYTE_PER_FRAME_MAX  	    (APP_AUDIO_TX_SAMPLE_PER_FRAME_MAX * APP_AUDIO_TX_BIT_PER_SAMPLE_MAX / 8)  // maximum number of byte per frame for the TX path (unit is frame)
#define APP_AUDIO_TX_SAMPLE_RATE_MAX            16000 			  														   // maximum sample rate supported for the TX path (unit is Hertz)
#if USB_MIC_ONLY
#define APP_AUDIO_TX_CHANNEL_NUMBER_MAX			4										   								   // maximum input channel of the TX path
#else
#define APP_AUDIO_TX_CHANNEL_NUMBER_MAX			4										   								   // maximum input channel of the TX path
#endif

/*
 * CODEC
 *  Rx output to speaker through codec WM8904
 */
#if CORE1
#include "fsl_wm8904.h"
#define APP_AUDIO_CODEC_SAMPLERATE				kWM8904_SampleRate16kHz
#define APP_AUDIO_CODEC_BITWIDTH				kWM8904_BitWidth32
#endif
/*
 * USB
 */
#define IRQ_PRIORITY_USB 						(1U)
// USB TX
#if USB_MIC_ONLY
#define APP_USB_TX_CHANNEL_MAX 	 				4																// Maximal number of channel in the USB Tx TODO
#else
#define APP_USB_TX_CHANNEL_MAX					4										   						// maximum input channel of the TX path
#endif
#define APP_USB_TX_BYTES_MAX 					APP_AUDIO_TX_BYTES_PER_SAMPLE_MAX								// Maximal number of bytes per samples in the USB Tx
#define APP_USB_TX_FORMAT_BITS_MAX				APP_AUDIO_TX_BIT_PER_SAMPLE_MAX									// Maximal number of bits per samples in the USB Tx
#define APP_USB_TX_SAMPLING_RATE_KHZ_MAX   	    (16000/1000)
#define APP_USB_TX_CIRCULAR_BUFFER_SIZE_BYTE    (APP_AUDIO_TX_BYTE_PER_FRAME_MAX *APP_USB_TX_CHANNEL_MAX * 5)
// USB RX
#define APP_USB_RX_CHANNEL_MAX 					2																// Maximal number of channel in the USB Rx
#define APP_USB_RX_BYTES_MAX 					APP_AUDIO_RX_BYTES_PER_SAMPLE_MAX								// Maximal number of bytes per samples in the USB Rx
#define APP_USB_RX_FORMAT_BITS_MAX				32																// Maximal number of bits per samples in the USB Rx
#define APP_USB_RX_FEEDBACK_SIZE_BYTE   		APP_USB_RX_CHANNEL_MAX * APP_USB_RX_BYTES_MAX   				// Maximal size allow for feedback calculation
#define APP_USB_RX_BYTE_PER_FRAME_MAX			APP_AUDIO_RX_SAMPLE_PER_FRAME_MAX * APP_USB_RX_BYTES_MAX
#define APP_USB_RX_SAMPLING_RATE_KHZ_MAX   		(16000/1000)
#define APP_USB_RX_CIRCULAR_BUFFER_SIZE_BYTE    (APP_AUDIO_RX_BYTE_PER_FRAME_MAX *APP_USB_RX_CHANNEL_MAX * 5)
/*
 * SW PROCESS
 */
#define APP_SW_PROCESS_MAX_SAMPLE_PER_FRAME		APP_MIN(APP_AUDIO_TX_SAMPLE_PER_FRAME_MAX,APP_AUDIO_RX_SAMPLE_PER_FRAME_MAX)
/*
 * User Button
 */
#define     BOARD_USER_BUTTON                   9
#define     BOARD_USER_BUTTON_NAME             "S3 (USER_BUTTON)"
/*
 * MAILBOX
 */
#define PRIMARY_CORE_MAILBOX_CPU_ID   			kMAILBOX_CM33_Core0
#define SECONDARY_CORE_MAILBOX_CPU_ID 			kMAILBOX_CM33_Core1
/*******************************************************************************
 * Enums
 ******************************************************************************/
// platform And Materials
typedef enum
{
    APP_PLATFORM_MATERIALS_LPC55S69,
	APP_PLATFORM_MATERIALS_LPC55S69_TB2136_RDSPMICARRAY,
/* CUSTOMER CONFIGURATION */
	APP_PLATFORM_MATERIALS_LPC55S69_CUSTOMER,
/*end CUSTOMER CONFIGURATION */
}APP_platformMaterials_en;


#endif /* APP_PLATFORM == */

#endif /* _APP_GLOBAL_PLATFORM_H_ */
