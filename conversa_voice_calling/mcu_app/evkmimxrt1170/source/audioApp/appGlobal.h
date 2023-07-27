/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _APP_GLOBAL_H_
#define _APP_GLOBAL_H_

#include "fsl_wm8960.h"

//FreeRtos
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

// Platform
#include "PL_platformTypes.h"

//audio
#include "audio.h"

// PDM
#include "fsl_pdm.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Application */
/*
 * Version info:
 *
 *    1.0.1 : initial version
 *
 * 			Platform:
 * 				- APP_PL_RT1170EVK_SCHC2_REVA
 * 				- APP_PL_RT1170EVK_SCHC3_REVA
 * 				- APP_PL_RT1170EVK_SCHC3_REVA_SHBOX => Steel head box
 *
 *          System:
 *              - FreeRTOS
 *
 *    		Audio frameWork:
 *    		    - USB Rx Tx stream
 *    		    - Audio Rx chain: SAI Tx with EDMA
 *    		    - Audio Tx chain: PDM Rx with EDMA
 *    		    - Loop back Tx source to Rx sink
 *    		    - MIPS measure with GPIO
 *
 *    		Process:
 *    			- Conversa (not supporting the SRC)
 *    			- Conversa tuning tool
 *    			- MIPS load simulation
 *
 *    1.1.0
 *
 * 			Optimization:
 * 				- Code is compiled with -O3
 * 				- Interrupt code instruction (EDM / PDM / SAI / USB audio flow & Vcom) are located into speed memory (FLEXRAM ITC for RT1170)
 * 				- Start capture mechanism is located into interrupt for better synchronization between Rx and Tx path
 *
 *    		Audio frameWork:
 * 				- Pre-load configuration (lb2 3 4 5 & usb2 3 4 5) are available
 * 				- Custom configuration is supported
 *
 *          System:
 *              - Force the scheduler to run or not run at IT exit after IT occurs according to IT synchronization order
 *              - decrease the scheduler tick rate configTICK_RATE_HZ to minimize system impact on MIPS consumption
 *
 * 			USB:
 * 				- USB VID PID updated
 *
 *    1.1.1
 *
 *    		Audio frameWork:
 * 				- Conversa pre-load configuration (conv2 3 4 5) are available
 *				- Force scheduler to run after Rx or Tx interrupt
 *				- Stop the scheduler for the start sequence SW code
 *
 *          System:
 *          	- Minimal frame size Tx or Rx = 16
 *
 *    		Process:
 *    		    - Check Conversa get data pointer return an address != NULL before use the pointer
 *
 *			others:
 *			    - Minor comment and printf update
 *			    - Minor code clean
 *
 *    1.1.2
 *
 *    		Audio frameWork:
 * 				- Add platform and materials application parameter
 *				- Auto load conversa parameters according to selected platform and materials
 *				- Support tangband speaker materials
 *
 *    1.2.0
 *    		Audio frameWork:
 *    		 	- Add UAC2.0 support
 *    		 	- add USB RX feedback & TX packet adjustment
 *
 *    1.2.1
 *    		Audio frameWork:
 *    		 	- remove unsupported configuration
 *    		 	- add SW7 (USER_BUTTON) control at start to run or shell task according shell task present compilation flag
 *    		 	- check Rx & Tx compatibility
 *    		Process:
 *    		    - add conversa swp3d15 & swp3d23 configuration
 *
 *    1.2.2
 *    		Audio frameWork:
 *				- All SWP config tested
 *    		Process:
 *    		    - Update conversa swp1 & swp2 configuration
 *
 *    1.2.3
 *    		Process:
 *    		    - Update conversa swp3 d35 & d45 configuration
 *    		    - Update conversa library to v5.1.1 (new tuning tool to be use)
 *
 *    1.2.4
 *    		Framework:
 *    		    - Rename tuning tool configuration file
 *    		Process:
 *    		    - Update conversa library to v5.1.2
 *
 *    1.2.5
 *    		Framework & configuration:
 *    		    - add preload command convshb2s32d23 for steelhead box amplifier wondom distance 2.3m (need to be tuned)
 *    		    - add preload command convtas32d100  for T.A distance 10m
 *
 *    1.2.6
 *    		 configuration:
 *    		    - update convshb2s32d23
 *    		    - update convswp3d15 d23 d35 d45 (d23 is matching teams tests)
 *    1.2.7
 *    		Framework:
 *    		    - Support last Conversa API
 *    		 configuration:
 *    		    - update convswp1d15 convswp2d15 convswp3d15 convswp3d23 (match teams test)
 *    1.2.8
 *    		Framework:
 *    		    - Solve aliasing issue when 1spk sink selected
 *    2.0.0
 *    		Framework:
 *    		    - Free RTOS update
 *    		    - Shell task handle by OSA
 *    		    - Shell task in non blocking mode
 *    2.0.1
 *    		Framework:
 *    		    - Update MIPS Measure by GPIO. Measure ISR and task timing
 *    		    - Fix issue with user button at start
 *    2.0.2
 *     		Framework:
 *    		    - Remove SDRAM memory mapping
 *    		    - Auto launch mode disable when Shell task present defined
 *     2.0.3 :
 *     		Frame work:
 *    		    - support frame size which is not a power of 2
 *    		    - Move to new Conversa utils and include repository structure
 *    		    - add new pre-load configuration to support conversa ML library
 *    2.1.0
 *          Frame work:
 *              - rename use case schell command
 *              - support new conversa parameter file API
 *          Conversa:
 *              - new library
 *              - new folder structure
  *    2.1.1
 *          Frame work:
 *              - minor shell command correction when CONVERSA_PRESENT not defined
 *
 */
#define APP_VERSION_MAJOR   			  		2				    // application major version
#define APP_VERSION_MINOR   			  		1				    // application minor version
#define APP_VERSION_PATCH   			  		0				    // application patch version

/* Default configuration */
#define SHELL_TASK_DEFAULT_CONFIGURATION 		"convswp32k"       // default configuration to be launched if shell task not present

/*
 * Hardware platform
 */
/* _CFG1 platform:
 *   			 APP_PL_RT1170EVK_SCHC1_REVX3
 *   			 APP_PL_RT1170EVK_SCHC2_REVA
 *    			 APP_PL_RT1170EVK_SCHC3_REVA
 *    			 APP_PL_RT1170EVK_SCHC4_REVA
 */
// RT1170 platform
#define APP_PL_RT1170EVK_SCHC1_REVX3			1
#define APP_PL_RT1170EVK_SCHC2_REVA				2
#define APP_PL_RT1170EVK_SCHC3_REVA				3
#define APP_PL_RT1170EVK_SCHC4_REVA				4

// USER platform
#define APP_PLATFORM_USER   	  		    	APP_PL_RT1170EVK_SCHC3_REVA  		    // Select the platform used

// SW code platform
#if ( (APP_PLATFORM_USER == APP_PL_RT1170EVK_SCHC1_REVX3) || (APP_PLATFORM_USER == APP_PL_RT1170EVK_SCHC2_REVA) ||(APP_PLATFORM_USER == APP_PL_RT1170EVK_SCHC3_REVA) || (APP_PLATFORM_USER == APP_PL_RT1170EVK_SCHC4_REVA))
	#define APP_PLATFORM 			APP_PL_RT1170EVK
	#define APP_PLATFORM_TEXT		"RT1170 EVK"
#else
	#error "ERROR: Platform not supported"
#endif

#include "appGlobal_platform.h" 	// include define which are platform dependent, keep this line bellow platform selection

/*
 *  Others
 */
#define APP_AUDIO_BLINK_LED_PERIOD_MS      				10000	// blink led period in ms
//#define APP_AUDIO_SHELL_CUSTOM_COMMAND_NOT_AUTHORIZED   		// To not authorized custom command and force user to use pre-load configuration

/* define the raw audio source bit per sample size */
// TODO Add condition if multi Tx input capability is present (ex: the SAI Rx)
#define APP_AUDIO_TX_RAW_BUF_BYTE_PER_FRAME_MAX	(APP_AUDIO_TX_SAMPLE_PER_FRAME_MAX * APP_AUDIO_PDM_BIT_PER_SAMPLE_MAX / 8)						// raw audio source bit per sample size is the PDM one as PDM is the unique TX input

/* CHECK AUDIO FRAME WORK COMPATIBLITY */
#if APP_AUDIO_PDM_MIC_NUMBER_MAX > APP_AUDIO_PDM_CHANNEL_NUMBER_MAX
	#error "ERROR APP_AUDIO_PDM_MIC_NUMBER_MAX > APP_AUDIO_PDM_CHANNEL_NUMBER_MAX"
#endif

#include "usb_audio_unified.h"

/*
 * Software define
 */
// Conversa
#ifdef CONVERSA_PRESENT 														// CONVERSA enable disable: Use pre-processor definition symbols to support or not conversa process (CONVERSA)
	#define CONVERSA_MEMORY_BYTE				 520000							// heap size in byte required by conversa library
	#define CONVERSA_TUNING_PARAM_BYTE			 4096							// size allocated for tuning param storing
#endif

/*******************************************************************************
 * Enums
 ******************************************************************************/

/*******************************************************************************
 * Structure
 ******************************************************************************/
/* Application handle structure */
typedef struct
{
	/* platform + materials used*/
	APP_platformMaterials_en     	platformAndMaterials;

	/* Shell task */
	TaskHandle_t 					shell_task_handle;

    /* AUDIO */
    AUDIO_definition_st 			audioDefinition;

    /* USB */
    usb_device_composite_struct_t 	usbTxRx_handle;

 }app_handle_st;

 /*******************************************************************************
 * Global Variables
 ******************************************************************************/
 // Source TX ping pong buffer
 SDK_ALIGN 	(  	extern PL_INT8 g_sourceRawBuffPingTx	[APP_AUDIO_PDM_CHANNEL_NUMBER_MAX * APP_AUDIO_TX_RAW_BUF_BYTE_PER_FRAME_MAX]   , sizeof(PL_UINT32)); // RAW (INTERLEAVED OR NOT) source buffer ping for the Tx path
 SDK_ALIGN 	(  	extern PL_INT8 g_sourceRawBuffPongTx	[APP_AUDIO_PDM_CHANNEL_NUMBER_MAX * APP_AUDIO_TX_RAW_BUF_BYTE_PER_FRAME_MAX]   , sizeof(PL_UINT32)); // RAW (INTERLEAVED OR NOT) source buffer pong for the Tx path

 // Source RX ping pong buffer
 SDK_ALIGN 	(  	extern PL_INT8 g_sourceRawBuffRx		[APP_AUDIO_RX_CHANNEL_NUMBER_MAX * APP_AUDIO_RX_BYTE_PER_FRAME_MAX] 		   , sizeof(PL_UINT32)); // RAW (INTERLEAVED OR NOT) source buffer for the Rx path

 // Sink RX ping pong buffer
 SDK_ALIGN 	(  	extern PL_INT8 g_sinkBuffPingRx			[APP_AUDIO_RX_CHANNEL_NUMBER_MAX * APP_AUDIO_RX_BYTE_PER_FRAME_MAX] 	   		, sizeof(PL_UINT32)); // sink buffer (INTERLEAVED OR NOT) ping for the Rx path
 SDK_ALIGN 	(  	extern PL_INT8 g_sinkBuffPongRx			[APP_AUDIO_RX_CHANNEL_NUMBER_MAX * APP_AUDIO_RX_BYTE_PER_FRAME_MAX] 	   		, sizeof(PL_UINT32)); // sink buffer (INTERLEAVED OR NOT) pong for the Rx path

 // Process buffer Tx and Rx
 SDK_ALIGN	(	extern PL_INT8 g_processBuff_Tx		    [APP_AUDIO_TX_CHANNEL_NUMBER_MAX] [APP_AUDIO_TX_BYTE_PER_FRAME_MAX]		   		, sizeof(PL_UINT32)); // Channel by Channel process buffer for the Tx path
 SDK_ALIGN	(	extern PL_INT8 g_processBuff_Rx		    [APP_AUDIO_RX_CHANNEL_NUMBER_MAX] [APP_AUDIO_RX_BYTE_PER_FRAME_MAX]		   		, sizeof(PL_UINT32)); // Channel by Channel process buffer for the Rx path

 /* USB Buffer */
 // USB capture buffer
 SDK_ALIGN 	(  	extern PL_INT8 g_USBBufferTx			[APP_USB_TX_CHANNEL_MAX] [APP_AUDIO_TX_BYTE_PER_FRAME_MAX] 		   			   	, sizeof(PL_UINT32)); // buffer used by the USB to store data to transmit to the USB Tx circular buffer at end of audio process loop

 // USB circular buffer
 SDK_ALIGN	(	extern PL_INT8 g_USBCircularBufferTX   [APP_USB_TX_CIRCULAR_BUFFER_SIZE_BYTE]									       	, sizeof(PL_UINT32)); // circular buffer used by the USB software
 SDK_ALIGN	(	extern PL_INT8 g_USBCircularBufferRX   [APP_USB_RX_CIRCULAR_BUFFER_SIZE_BYTE]									       	, sizeof(PL_UINT32)); // circular buffer used by the USB software

/*
 *  Software buffer
 */
// Conversa
#ifdef CONVERSA_PRESENT
SDK_ALIGN	( extern PL_INT8  g_conversaMemory		[CONVERSA_MEMORY_BYTE]								   				    			, sizeof(PL_UINT32)); // memory reserved for conversa
#endif

// application handle
extern app_handle_st g_appHandle;

/*
 * PDM
 */
extern pdm_config_t 		 g_pdmConfig;  		  // PDM IP general configuration
extern pdm_channel_config_t  g_pdmChannelConfig;  // PDM config for each channel

/*
 * SAI
 */
extern sai_edma_handle_t	g_saiTxEdmaHandle;    // Sai Tx Edma handle used to manage the SAI TX + associated EDMA
extern edma_handle_t 		g_dmaSaiTxHandle;	  // Edma handle for the SAI TX

// Others

/*
 *  debug
 */
// extern PL_UINT32 g_iDebug1;

/*
 *  Tx Rx synchro
 */
extern volatile PL_UINT32 					g_startMechanismTxRx;	// to synchronise Rx and Tx at start

#endif /* _APP_GLOBAL_H_ */
