
/* Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _APP_GLOBAL_H_
#define _APP_GLOBAL_H_


// Platform
#include "PL_platformTypes.h"
#include "audio.h"
//audio
#ifdef CORE1
#include "usb_audio_unified.h"
#endif
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Application */
/*
 * Version info:
 *
 *	1.0.1 : initial version
 *
 * 			Platform:
 * 				- LPC55S69Evk
 *
 *          System:
 *              - No FreeRTOS
 *
 *    		Audio frameWork:
 *    		    - USB Rx Tx stream
 *    		    - Audio Rx chain: SAI Tx with EDMA
 *    		    - Audio Tx chain: PDM Rx with EDMA
 *    		    - Loop back Tx source to Rx sink
 *    		    - MIPS measure with GPIO
 *
 *    		Process:
 *    			- PDM converting (optional)
 *    			- Conversa
 *    			- Conversa tuning tool
 *    			- MIPS load simulation
 *
 * 	1.1.0 : SWP Compliant
 *
 *    		Audio frameWork:
 *    		    - framesize can be changed
 *    		    - DMA & I2S transfer are 1 framesize long
 *    		    - Mailbox used to communicate CORE0<->CORE1
 *    		    - Circular buffer used for USB rx/tx
 *    		    - RXTX process rework
 *
 * 	1.1.1 : SWP Compliant 2 Major Updates
 *
 *    		Audio frameWork:
 *    		- Put ISR code in RAM to reduce ISR time spent in
 *    		- Add CORE0 check status before Conversa call to ensure that previous frame has been successfully processed
 *    		- change GPIO use
 *    		- rework Ping/Pong mechanism
 */

#define APP_VERSION_MAJOR   			  		1				    // application major version
#define APP_VERSION_MINOR   			  		1				    // application minor version
#define APP_VERSION_PATCH   			  		1				    // application patch version

// USER platform
#define APP_PLATFORM_USER   	  		    	APP_PL_LPC55S69EVK_SCHA3_REVA3 		    // Select the platform used
/* Default configuration */
#define SHELL_COMMAND_DEFAULT_CONFIGURATION 	"spswp16k"            // default configuration to be launched if shell commands not present
/*
 * Hardware platform
 */
/* _CFG1 platform:
 *   			 APP_PL_LPC55S69EVK_SCHA3_REVA3
 */

// application handle
#if (APP_PLATFORM_USER == APP_PL_LPC55S69EVK_SCHA3_REVA3)
#define APP_PLATFORM 							APP_PL_LPC55S69EVK
#define APP_PLATFORM_TEXT						"LPC55S69 EVK"
#else
	#error "ERROR: Platform not supported"
#endif

#include "appGlobal_platform.h" 	// include define which are platform dependent, keep this line bellow platform selection



/*******************************************************************************
 * Structure
 ******************************************************************************/
/* Structure used by Framework (core1)
/* Application handle structure */
#ifdef CORE1
typedef struct
{
	/* platform + materials used*/
	APP_platformMaterials_en     	platformAndMaterials;
    /* AUDIO */
    AUDIO_definition_st 			audioDefinition;
    /* USB */
    USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
    usb_device_composite_struct_t 	UsbCompositeDev_handle;
 }app_handle_st;
 /*
  * Tx Rx synchro
  */
 extern app_handle_st g_appHandle;
 extern usb_device_composite_struct_t *g_UsbCompositeDevPtr;
#endif
 /* Shared structure between CORE0 & CORE1 */
#ifdef CORE0
#include"tools.h"
#endif
 typedef struct
 {
	 // Conversa Process buffers TX
	 PL_INT8 pp_ConversaProcessBuff_TxIn [2][APP_AUDIO_TX_CHANNEL_NUMBER_MAX]  [APP_AUDIO_TX_BYTE_PER_FRAME_MAX];
	 PL_INT8 pp_ConversaProcessBuff_TxOut[2][1]                                [APP_AUDIO_TX_BYTE_PER_FRAME_MAX];
#if RX_PATH_PRESENT
	 // Conversa Process buffers TX
	 PL_INT8 pp_ConversaProcessBuff_RxIn [2][1]                                [APP_AUDIO_RX_BYTE_PER_FRAME_MAX];
	 PL_INT8 pp_ConversaProcessBuff_RxOut[2][2]                                [APP_AUDIO_RX_BYTE_PER_FRAME_MAX];
#endif
	 // Conversa Process FLOAT buffers TX
	 PL_FLOAT pp_ConversaProcessBuff_TxIn_Flt [APP_AUDIO_TX_CHANNEL_NUMBER_MAX][APP_AUDIO_TX_SAMPLE_PER_FRAME_MAX];
	 PL_FLOAT pp_ConversaProcessBuff_TxOut_Flt[1]                              [APP_AUDIO_TX_SAMPLE_PER_FRAME_MAX];
#if RX_PATH_PRESENT
	 // Conversa Process FLOAT buffers TX
	 PL_FLOAT pp_ConversaProcessBuff_RxIn_Flt [1]							   [APP_AUDIO_RX_SAMPLE_PER_FRAME_MAX];
	 PL_FLOAT pp_ConversaProcessBuff_RxOut_Flt[1]                              [APP_AUDIO_RX_SAMPLE_PER_FRAME_MAX];
#endif
	 // Buffer selector
 	 PL_UINT32                      buffToChoose;
 	 // Audio path
 	AUDIO_definition_st 			audioDefinition;

 } T_CommonVarSharedByCore0AndCore1;
 /*******************************************************************************
  * Global Variables
  ******************************************************************************/
#if RX_PATH_PRESENT
 // Sink RX ping pong buffer
 SDK_ALIGN 	( extern PL_UINT32 g_sinkBuffPingRx	      [APP_AUDIO_RX_SAMPLE_PER_FRAME_MAX * APP_AUDIO_RX_CHANNEL_NUMBER_MAX] 	  	, sizeof(PL_UINT16)); // sink buffer (INTERLEAVED OR NOT) ping for the Rx path
 SDK_ALIGN 	( extern PL_UINT32 g_sinkBuffPongRx	      [APP_AUDIO_RX_SAMPLE_PER_FRAME_MAX * APP_AUDIO_RX_CHANNEL_NUMBER_MAX] 	   	, sizeof(PL_UINT16)); // sink buffer (INTERLEAVED OR NOT) pong for the Rx path
#endif
 // Source TX ping pong buffer
 SDK_ALIGN 	( extern PL_UINT32 g_sourceRawBuffPingTx  [APP_AUDIO_TX_SAMPLE_PER_FRAME_RDSPMIC * APP_AUDIO_I2S_TX_MAX_MICS_PER_CH] 	   	, sizeof(PL_UINT16)); // sink buffer (INTERLEAVED OR NOT) ping for the Rx path
 SDK_ALIGN 	( extern PL_UINT32 g_sourceRawBuffPongTx  [APP_AUDIO_TX_SAMPLE_PER_FRAME_RDSPMIC * APP_AUDIO_I2S_TX_MAX_MICS_PER_CH]	   	, sizeof(PL_UINT16)); // sink buffer (INTERLEAVED OR NOT) pong for the Rx path
 // USB Circular buffers
 SDK_ALIGN 	( extern PL_INT8   g_USBCircularBufferTX  [APP_USB_TX_CIRCULAR_BUFFER_SIZE_BYTE] 	   	                                , sizeof(PL_UINT16)); // usb tx circular buffer
#if RX_PATH_PRESENT
 SDK_ALIGN 	( extern PL_INT8   g_USBCircularBufferRX  [APP_USB_RX_CIRCULAR_BUFFER_SIZE_BYTE] 	   	                                , sizeof(PL_UINT16)); // usb rx circular buffer
#endif
 // Process buffer Tx and Rx
 SDK_ALIGN 	( extern PL_INT32  g_TXRawBuff	          [APP_AUDIO_TX_SAMPLE_PER_FRAME_RDSPMIC * APP_AUDIO_I2S_TX_MAX_MICS_PER_CH] 	, sizeof(PL_UINT16)); // usb rx circular buffer
#if RX_PATH_PRESENT
 SDK_ALIGN 	( extern PL_INT8   g_processBuff_Rx	      [APP_USB_RX_CHANNEL_MAX]      [APP_AUDIO_RX_BYTE_PER_FRAME_MAX] 	   			, sizeof(PL_UINT16));
#endif
 SDK_ALIGN	( extern PL_INT8   g_processBuff_Tx		  [APP_USB_TX_CHANNEL_MAX]      [APP_AUDIO_TX_BYTE_PER_FRAME_MAX]				, sizeof(PL_UINT16)); // Channel by Channel process buffer for the Tx path// usb rx circular buffer
 // for SSRC use
 SDK_ALIGN 	( extern PL_INT32   g_TXRawBuff	          [APP_AUDIO_TX_SAMPLE_PER_FRAME_RDSPMIC * APP_AUDIO_I2S_TX_MAX_MICS_PER_CH] 																	    , sizeof(PL_UINT16)); // usb rx circular buffer
 SDK_ALIGN 	( extern PL_INT32	 SSRC_scratchMem      [APP_AUDIO_TX_SAMPLE_PER_FRAME_RDSPMIC * 6] 					                , sizeof(PL_UINT16)); // SSRC scratch memory


 extern volatile PL_BOOL					  g_ConversaIsInit;
#endif /* _APP_GLOBAL_H_ */
