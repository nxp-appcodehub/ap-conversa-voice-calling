/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// Standard
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

//platform
#include "PL_platformTypes.h"

// Shell
#include "commandShell.h"
#include "fsl_debug_console.h"
#include "fsl_shell.h"

//Audio
#include "audio.h"

// Application
#include "appGlobal.h"

// init
#include "init.h"

#ifdef CONVERSA_PRESENT
	#include "ConversaTuningConfig.h"
#endif

#if (APP_PLATFORM == APP_PL_RT1170EVK)
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
PL_INT8 loopBackConfigSecurityStep = PL_FALSE; // to secure the user hit 2 times the loopback command and read the warning message

/*******************************************************************************
 * Static function
 ******************************************************************************/
static void loopBackModeSecurityLoop(void);

/*******************************************************************************
 * Code
 ******************************************************************************/

/* shellTxRxPathDef
 *  - decode the text chain
 *  - see commandShell.c for information
 */
shell_status_t shellTxRxPathDef(shell_handle_t shellHandle, int32_t argc, char **argv)
{
	// error handle
	shell_status_t shellRetStatus 	 = kStatus_SHELL_Success;
	status_t 	   retStatusFunc	 = kStatus_Success;

	// parameters
	AUDIO_definition_st*     p_audioDefinition_handle 		= &g_appHandle.audioDefinition;
	AUDIO_configParam_st*    p_audioTxConfigParam 	  		= &g_appHandle.audioDefinition.audioTxPath_handle.configParam;
	AUDIO_configParam_st*    p_audioRxConfigParam 	  		= &g_appHandle.audioDefinition.audioRxPath_handle.configParam;
	AUDIO_workFlowHandle_st* p_audioWorkFlowHandle 			= &g_appHandle.audioDefinition.audioWorkFlow_handle;
	AUDIO_process_en*		 p_audioProcessList	  	  		= &g_appHandle.audioDefinition.processList[0];

	// decode and action handle
	volatile PL_INT16 iProcess 	 = 0; 		 // counter for process list
	volatile PL_INT16 iDecode  	 = 0; 		 // counter for decoder

	// Others
	PL_INT32 temp1_32b 		   = 0; 	// temporary 32 bit value
	PL_INT32 temp2_32b 		   = 0; 	// temporary 32 bit value

	// defaults configs handle
	char cmdDefault[20][20]; 			// command char chain used for default configs

	/*
	 * Reset configuration value
	 */
	for (iDecode=0 ; iDecode < AUDIO_PROCESS_LIST_MAX; iDecode++)
	{
		p_audioProcessList[iDecode] = AUDIO_PROC_NO;
	}

	/*
	 *   Force chain to be in lower case for future comparison
	 */
	for (iDecode=1; iDecode < argc; iDecode++)							  // parse each string
	{
		for (iProcess = 0; iProcess < strlen(argv[iDecode]); iProcess++ ) // Parse each char of the string
		{
			argv[iDecode][iProcess] = tolower(argv[iDecode][iProcess]);   // convert to lower case
		}
	}

	/*Protection to force reset between 2 configurations
	 *    Check if a configuration is already running
	 */
	if ( p_audioWorkFlowHandle->processEventGroup != NULL)
	{
		PRINTF("FAIL: Audio configuration is already running, please type 'reset' command before select a new configuration\r\n\n");
		return kStatus_SHELL_Error;
	}
	/*
	 * Common configuration
	 */
	p_audioWorkFlowHandle->startOrder = AUDIO_START_ORDER_SAITX_PDM; // start SAITx first then PDM

	/*
	 * set sample rate or default configuration
	 */
	iDecode = 1;
	if (argc > iDecode)
	{
		/* SOFTWARE PACK CONFIGURATION */
		/* All SW pack configuration for 16K */
		// Loop back 16K
		if (strcmp(argv[iDecode], "lbswp16k") == 0)
		{
			if ( loopBackConfigSecurityStep == PL_FALSE )	// if 1st time we ask for loopback command
			{
				loopBackModeSecurityLoop();
				return kStatus_SHELL_Success;
			}
			else
			{
				PRINTF("\nUse case: Loop back between microphones and headset + Usb microphones. Fs=16kHz\r\n");
				argc = 9;
				strcpy(&cmdDefault[0][0],  "rt1170");     		// platform and materials
				strcpy(&cmdDefault[1][0],  "16000");            // frame rate in Hz
				strcpy(&cmdDefault[2][0],  "64");				// sample per frame
				strcpy(&cmdDefault[3][0],  "32");				// bit per sample
				strcpy(&cmdDefault[4][0],  "3dmic");	        // Tx source
				strcpy(&cmdDefault[5][0],  "usb4");				// Tx sink
				strcpy(&cmdDefault[6][0],  "tx0_1");			// Rx source
				strcpy(&cmdDefault[7][0],  "2spk");			    // Rx sink
				strcpy(&cmdDefault[8][0],  "no");				// process
				// print the in line command
				PRINTF("\t( ");
				for (iDecode=0 ; iDecode < argc ; iDecode++)
				{
					argv[iDecode] = &cmdDefault[iDecode][0];
					PRINTF("%s ", argv[iDecode]);
				}
				PRINTF(")\r\n\n");
				iDecode = 0; // set iDecode to 0 for the following decoding command
			}
		}
		// USB 16K
		else if (strcmp(argv[iDecode], "usbswp16k") == 0)
		{
			PRINTF("\nUse case: Usb microphones and Usb speaker. Fs=16kHz");
			argc = 9;
			strcpy(&cmdDefault[0][0],  "rt1170");  			// platform and materials
			strcpy(&cmdDefault[1][0],  "16000");            // frame rate in Hz
			strcpy(&cmdDefault[2][0],  "64");				// sample per frame
			strcpy(&cmdDefault[3][0],  "32");				// bit per sample
			strcpy(&cmdDefault[4][0],  "3dmic");	        // Tx source
			strcpy(&cmdDefault[5][0],  "usb4");				// Tx sink
			strcpy(&cmdDefault[6][0],  "usb1");				// Rx source
			strcpy(&cmdDefault[7][0],  "1spk");			    // Rx sink
			strcpy(&cmdDefault[8][0],  "no");				// process
			// print the in line command
			PRINTF("\t( ");
			for (iDecode=0 ; iDecode < argc ; iDecode++)
			{
				argv[iDecode] = &cmdDefault[iDecode][0];
				PRINTF("%s ", argv[iDecode]);
			}
			PRINTF(")\r\n\n");
			iDecode = 0; // set iDecode to 0 for the following decoding command
		}
		// Speaker Phone 16K
		else if (strcmp(argv[iDecode], "spswp16k") == 0)
		{
			PRINTF("\nUse case: Speaker phone with Conversa software (required voice call RT1170 mockup). Teams conferencing device 2.3m. Fs=16kHz.");
			argc = 9;
			strcpy(&cmdDefault[0][0],  "rt1170tb2136");  // platform and materials
			strcpy(&cmdDefault[1][0],  "16000");            // frame rate in Hz
			strcpy(&cmdDefault[2][0],  "64");				// sample per frame
			strcpy(&cmdDefault[3][0],  "32");				// bit per sample
			strcpy(&cmdDefault[4][0],  "3dmic");	        // Tx source
			strcpy(&cmdDefault[5][0],  "usb4");				// Tx sink
			strcpy(&cmdDefault[6][0],  "usb1");				// Rx source
			strcpy(&cmdDefault[7][0],  "1spk");			    // Rx sink
			strcpy(&cmdDefault[8][0],  "conversaswp16k");   // process
			// print the in line command
			PRINTF("\t( ");
			for (iDecode=0 ; iDecode < argc ; iDecode++)
			{
				argv[iDecode] = &cmdDefault[iDecode][0];
				PRINTF("%s ", argv[iDecode]);
			}
			PRINTF(")\r\n\n");
			iDecode = 0; // set iDecode to 0 for the following decoding command
		}

		/* All SW pack configuration for 32K */
		// Loop back 32K
		else if (strcmp(argv[iDecode], "lbswp32k") == 0)
		{
			if ( loopBackConfigSecurityStep == PL_FALSE )	// if 1st time we ask for loopback command
			{
				loopBackModeSecurityLoop();
				return kStatus_SHELL_Success;
			}
			else
			{
				PRINTF("\nUse case: Loop back between microphones and headset + Usb microphones. Fs=32kHz");
				argc = 9;
				strcpy(&cmdDefault[0][0],  "rt1170");  			// platform and materials
				strcpy(&cmdDefault[1][0],  "32000");            // frame rate in Hz
				strcpy(&cmdDefault[2][0],  "128");				// sample per frame
				strcpy(&cmdDefault[3][0],  "32");				// bit per sample
				strcpy(&cmdDefault[4][0],  "3dmic");	        // Tx source
				strcpy(&cmdDefault[5][0],  "usb4");				// Tx sink
				strcpy(&cmdDefault[6][0],  "tx0_1");			// Rx source
				strcpy(&cmdDefault[7][0],  "2spk");			    // Rx sink
				strcpy(&cmdDefault[8][0],  "no");				// process
				// print the in line command
				PRINTF("\t( ");
				for (iDecode=0 ; iDecode < argc ; iDecode++)
				{
					argv[iDecode] = &cmdDefault[iDecode][0];
					PRINTF("%s ", argv[iDecode]);
				}
				PRINTF(")\r\n\n");
				iDecode = 0; // set iDecode to 0 for the following decoding command
			}
		}
		// USB 32K
		else if (strcmp(argv[iDecode], "usbswp32k") == 0)
		{
			PRINTF("\nUse case: Usb microphones and Usb speaker. Fs=32kHz");
			argc = 9;
			strcpy(&cmdDefault[0][0],  "rt1170");    		// platform and materials
			strcpy(&cmdDefault[1][0],  "32000");            // frame rate in Hz
			strcpy(&cmdDefault[2][0],  "128");				// sample per frame
			strcpy(&cmdDefault[3][0],  "32");				// bit per sample
			strcpy(&cmdDefault[4][0],  "3dmic");	        // Tx source
			strcpy(&cmdDefault[5][0],  "usb4");				// Tx sink
			strcpy(&cmdDefault[6][0],  "usb1");				// Rx source
			strcpy(&cmdDefault[7][0],  "1spk");			    // Rx sink
			strcpy(&cmdDefault[8][0],  "no");				// process
			// print the in line command
			PRINTF("\t( ");
			for (iDecode=0 ; iDecode < argc ; iDecode++)
			{
				argv[iDecode] = &cmdDefault[iDecode][0];
				PRINTF("%s ", argv[iDecode]);
			}
			PRINTF(")\r\n\n");
			iDecode = 0; // set iDecode to 0 for the following decoding command
		}
		// Speaker Phone 32K
		else if (strcmp(argv[iDecode], "spswp32k") == 0)  // Configuration is microsoft teams compliant
		{
			PRINTF("\nUse case: Speaker phone with Conversa software (required voice call RT1170 mockup). Teams conferencing device 2.3m. Fs=32kHz.");
			argc = 9;
			strcpy(&cmdDefault[0][0],  "rt1170tb2136");     // platform and materials
			strcpy(&cmdDefault[1][0],  "32000");            // frame rate in Hz
			strcpy(&cmdDefault[2][0],  "128");				// sample per frame
			strcpy(&cmdDefault[3][0],  "32");				// bit per sample
			strcpy(&cmdDefault[4][0],  "3dmic");	        // Tx source
			strcpy(&cmdDefault[5][0],  "usb4");				// Tx sink
			strcpy(&cmdDefault[6][0],  "usb1");				// Rx source
			strcpy(&cmdDefault[7][0],  "1spk");			    // Rx sink
			strcpy(&cmdDefault[8][0],  "conversaswp32k");		    // process
			// print the in line command
			PRINTF("\t( ");
			for (iDecode=0 ; iDecode < argc ; iDecode++)
			{
				argv[iDecode] = &cmdDefault[iDecode][0];
				PRINTF("%s ", argv[iDecode]);
			}
			PRINTF(")\r\n\n");
			iDecode = 0; // set iDecode to 0 for the following decoding command
		}
		/* end SOFTWARE PACK CONFIGURATION */

		/* NXP INTERNAL CONFIGURATION */
		// Loop back 24K
		else if (strcmp(argv[iDecode], "lbswp24k") == 0)
		{
			if ( loopBackConfigSecurityStep == PL_FALSE )	// if 1st time we ask for loopback command
			{
				loopBackModeSecurityLoop();
				return kStatus_SHELL_Success;
			}
			else
			{
				PRINTF("\nUse case: Loop back between microphones and headset + Usb microphones. Fs=24kHz");
				argc = 9;
				strcpy(&cmdDefault[0][0],  "rt1170");  			// platform and materials
				strcpy(&cmdDefault[1][0],  "24000");            // frame rate in Hz
				strcpy(&cmdDefault[2][0],  "64");				// sample per frame
				strcpy(&cmdDefault[3][0],  "32");				// bit per sample
				strcpy(&cmdDefault[4][0],  "3dmic");	        // Tx source
				strcpy(&cmdDefault[5][0],  "usb4");				// Tx sink
				strcpy(&cmdDefault[6][0],  "tx0_1");			// Rx source
				strcpy(&cmdDefault[7][0],  "2spk");			    // Rx sink
				strcpy(&cmdDefault[8][0],  "no");				// process
				// print the in line command
				PRINTF("\t( ");
				for (iDecode=0 ; iDecode < argc ; iDecode++)
				{
					argv[iDecode] = &cmdDefault[iDecode][0];
					PRINTF("%s ", argv[iDecode]);
				}
				PRINTF(")\r\n\n");
				iDecode = 0; // set iDecode to 0 for the following decoding command
			}
		}
		else if (strcmp(argv[iDecode], "usbswp24k") == 0)
		{
			PRINTF("\nUse case: Usb microphones and Usb speaker. Fs=24kHz");
			argc = 9;
			strcpy(&cmdDefault[0][0],  "rt1170");  			// platform and materials
			strcpy(&cmdDefault[1][0],  "24000");            // frame rate in Hz
			strcpy(&cmdDefault[2][0],  "64");				// sample per frame
			strcpy(&cmdDefault[3][0],  "32");				// bit per sample
			strcpy(&cmdDefault[4][0],  "3dmic");	        // Tx source
			strcpy(&cmdDefault[5][0],  "usb4");				// Tx sink
			strcpy(&cmdDefault[6][0],  "usb1");				// Rx source
			strcpy(&cmdDefault[7][0],  "1spk");			    // Rx sink
			strcpy(&cmdDefault[8][0],  "no");				// process
			// print the in line command
			PRINTF("\t( ");
			for (iDecode=0 ; iDecode < argc ; iDecode++)
			{
				argv[iDecode] = &cmdDefault[iDecode][0];
				PRINTF("%s ", argv[iDecode]);
			}
			iDecode = 0; // set iDecode to 0 for the following decoding command
		}
		else if (strcmp(argv[iDecode], "spswp24k") == 0)
		{
			PRINTF("\nUse case: Speaker phone with Conversa software (required voice call RT1170 mockup). Teams conferencing device 2.3m. Fs=24kHz.");
			argc = 9;
			strcpy(&cmdDefault[0][0],  "rt1170tb2136");  // platform and materials
			strcpy(&cmdDefault[1][0],  "24000");            // frame rate in Hz
			strcpy(&cmdDefault[2][0],  "64");				// sample per frame
			strcpy(&cmdDefault[3][0],  "32");				// bit per sample
			strcpy(&cmdDefault[4][0],  "3dmic");	        // Tx source
			strcpy(&cmdDefault[5][0],  "usb4");				// Tx sink
			strcpy(&cmdDefault[6][0],  "usb1");				// Rx source
			strcpy(&cmdDefault[7][0],  "1spk");			    // Rx sink
			strcpy(&cmdDefault[8][0],  "conversaswp24k");			// process
			// print the in line command
			PRINTF("\t( ");
			for (iDecode=0 ; iDecode < argc ; iDecode++)
			{
				argv[iDecode] = &cmdDefault[iDecode][0];
				PRINTF("%s ", argv[iDecode]);
			}
			PRINTF(")\r\n\n");
			iDecode = 0; // set iDecode to 0 for the following decoding command
		}
		/* All SW pack configuration for 16K ML configuration */
		else if (strcmp(argv[iDecode], "lbswp16k_ml") == 0)
		{
			if ( loopBackConfigSecurityStep == PL_FALSE )	// if 1st time we ask for loopback command
			{
				loopBackModeSecurityLoop();
				return kStatus_SHELL_Success;
			}
			else
			{
				PRINTF("\nUse case: Loop back between microphones and headset + Usb microphones. Fs=16kHz)");
				argc = 9;
				strcpy(&cmdDefault[0][0],  "rt1170");     		// platform and materials
				strcpy(&cmdDefault[1][0],  "16000");            // frame rate in Hz
				strcpy(&cmdDefault[2][0],  "120");				// sample per frame
				strcpy(&cmdDefault[3][0],  "32");				// bit per sample
				strcpy(&cmdDefault[4][0],  "3dmic");	        // Tx source
				strcpy(&cmdDefault[5][0],  "usb4");				// Tx sink
				strcpy(&cmdDefault[6][0],  "tx0_1");			// Rx source
				strcpy(&cmdDefault[7][0],  "2spk");			    // Rx sink
				strcpy(&cmdDefault[8][0],  "no");				// process
				// print the in line command
				PRINTF("\t( ");
				for (iDecode=0 ; iDecode < argc ; iDecode++)
				{
					argv[iDecode] = &cmdDefault[iDecode][0];
					PRINTF("%s ", argv[iDecode]);
				}
				PRINTF(")\r\n\n");
				iDecode = 0; // set iDecode to 0 for the following decoding command
			}
		}
		else if (strcmp(argv[iDecode], "usbswp16k_ml") == 0)
		{
			PRINTF("\nUse case: Usb microphones and Usb speaker. Fs=16kHz");
			argc = 9;
			strcpy(&cmdDefault[0][0],  "rt1170");  			// platform and materials
			strcpy(&cmdDefault[1][0],  "16000");            // frame rate in Hz
			strcpy(&cmdDefault[2][0],  "120");				// sample per frame
			strcpy(&cmdDefault[3][0],  "32");				// bit per sample
			strcpy(&cmdDefault[4][0],  "3dmic");	        // Tx source
			strcpy(&cmdDefault[5][0],  "usb4");				// Tx sink
			strcpy(&cmdDefault[6][0],  "usb1");				// Rx source
			strcpy(&cmdDefault[7][0],  "1spk");			    // Rx sink
			strcpy(&cmdDefault[8][0],  "no");				// process
			// print the in line command
			PRINTF("\t( ");
			for (iDecode=0 ; iDecode < argc ; iDecode++)
			{
				argv[iDecode] = &cmdDefault[iDecode][0];
				PRINTF("%s ", argv[iDecode]);
			}
			PRINTF(")\r\n\n");
			iDecode = 0; // set iDecode to 0 for the following decoding command
		}
		else if (strcmp(argv[iDecode], "spswp16k_ml") == 0)
		{
			PRINTF("\nUse case: Speaker phone with Conversa ML software (required voice call RT1170 mockup). Teams conferencing device 2.3m. Fs=16kHz.");
			argc = 9;
			strcpy(&cmdDefault[0][0],  "rt1170tb2136d15");  // platform and materials
			strcpy(&cmdDefault[1][0],  "16000");            // frame rate in Hz
			strcpy(&cmdDefault[2][0],  "120");				// sample per frame
			strcpy(&cmdDefault[3][0],  "32");				// bit per sample
			strcpy(&cmdDefault[4][0],  "3dmic");	        // Tx source
			strcpy(&cmdDefault[5][0],  "usb4");				// Tx sink
			strcpy(&cmdDefault[6][0],  "usb1");				// Rx source
			strcpy(&cmdDefault[7][0],  "1spk");			    // Rx sink
			strcpy(&cmdDefault[8][0],  "conversaswp16k_ml");// process
			// print the in line command
			PRINTF("\t( ");
			for (iDecode=0 ; iDecode < argc ; iDecode++)
			{
				argv[iDecode] = &cmdDefault[iDecode][0];
				PRINTF("%s ", argv[iDecode]);
			}
			PRINTF(")\r\n\n");
			iDecode = 0; // set iDecode to 0 for the following decoding command
		}
		/* mockup => Steelhead + wondom amplifier TA2024 configuration */
		else if (strcmp(argv[iDecode], "spshb32k") == 0)
		{
			PRINTF("TEST Voice call speaker phone (Conversa) at 32K / 2.3m - steelhead box");
			argc = 9;
			strcpy(&cmdDefault[0][0],  "rt1170shbamp2"); 	// platform and materials
			strcpy(&cmdDefault[1][0],  "32000");            // frame rate in Hz
			strcpy(&cmdDefault[2][0],  "128");				// sample per frame
			strcpy(&cmdDefault[3][0],  "32");				// bit per sample
			strcpy(&cmdDefault[4][0],  "3dmic");	        // Tx source
			strcpy(&cmdDefault[5][0],  "usb4");				// Tx sink
			strcpy(&cmdDefault[6][0],  "usb1");				// Rx source
			strcpy(&cmdDefault[7][0],  "1spk");				// Rx sink
			strcpy(&cmdDefault[8][0],  "conversashb32k"); 	// process
			// print the in line command
			PRINTF("\t( ");
			for (iDecode=0 ; iDecode < argc ; iDecode++)
			{
				argv[iDecode] = &cmdDefault[iDecode][0];
				PRINTF("%s ", argv[iDecode]);
			}
			PRINTF(")\r\n\n");
			iDecode = 0; // set iDecode to 0 for the following decoding command
		}
		/*end NXP INTERNAL CONFIGURATION */


#ifdef APP_AUDIO_SHELL_CUSTOM_COMMAND_NOT_AUTHORIZED  		// To not authorized custom command and force user to use pre-load configuration
	else
	{
		PRINTF("FAIL: %s configuration not available, please select a pre-load config\r\n",argv[iDecode]);
		return kStatus_SHELL_Error;
	}
#endif

		/* set the loop back configuration protection */
		loopBackConfigSecurityStep = PL_TRUE;

		/*
		 * Start decode command
		 */

		if (argc > iDecode)
		{
			/*
			 *  set platform
			 */
			if (strcmp(argv[iDecode], "rt1170") == 0)
			{
				g_appHandle.platformAndMaterials = APP_PLATFORM_MATERIALS_RT1170;
			}
			else if (strcmp(argv[iDecode], "rt1170tb2136") == 0)
			{
				g_appHandle.platformAndMaterials = APP_PLATFORM_MATERIALS_RT1170_TB2136;
			}
			else if (strcmp(argv[iDecode], "rt1170shbamp2") == 0)
			{
				g_appHandle.platformAndMaterials = APP_PLATFORM_MATERIALS_RT1170_SHB_AMP2;
			}
			else
			{
				PRINTF("FAIL: %s is not a valid choice\r\n",argv[iDecode]);
				return kStatus_SHELL_Error;
			}
		}
	}

	/*
	 *  set sample rate
	 */
	iDecode = iDecode + 1;
	if (argc > iDecode)
	{
		if (strcmp(argv[iDecode], "16000") == 0)
		{
			p_audioTxConfigParam->sampleRate = 16000;
			p_audioRxConfigParam->sampleRate = 16000;
		}
		else if (strcmp(argv[iDecode], "24000") == 0)
		{
			p_audioTxConfigParam->sampleRate = 24000;
			p_audioRxConfigParam->sampleRate = 24000;
		}
		else if (strcmp(argv[iDecode], "32000") == 0)
		{
			p_audioTxConfigParam->sampleRate = 32000;
			p_audioRxConfigParam->sampleRate = 32000;
		}
		else if (strcmp(argv[iDecode], "48000") == 0)
		{
			p_audioTxConfigParam->sampleRate = 48000;
			p_audioRxConfigParam->sampleRate = 48000;
		}
		else
		{
			PRINTF("FAIL: %s is not a valid choice\r\n",argv[iDecode]);
			return kStatus_SHELL_Error;
		}
	}

	/*
	 * set frame size
	 */
	iDecode = iDecode + 1;
	if (argc > iDecode)
	{
		temp1_32b = atoi(argv[iDecode]);				 // string to int conversion

		// TX & RX path
		if (   (temp1_32b >= APP_AUDIO_TX_SAMPLE_PER_FRAME_MIN)
			&& (temp1_32b <= APP_AUDIO_TX_SAMPLE_PER_FRAME_MAX)
		   )
		{
			p_audioTxConfigParam->samplePerFrame = temp1_32b;
			p_audioRxConfigParam->samplePerFrame = temp1_32b;
		}
		else
		{
			PRINTF("FAIL: %s is not a valid choice ( not in the acceptable range )\r\n",argv[iDecode]);
			return kStatus_SHELL_Error;
		}
	}

	/*
	 * set bit per sample
	 */
	iDecode = iDecode + 1;
	if (argc > iDecode)
	{
		// TX path
		if (strcmp(argv[iDecode], "16") == 0)
		{
			p_audioTxConfigParam->bitPerSample = 16;
			p_audioRxConfigParam->bitPerSample = 16;
			g_appHandle.usbTxRx_handle.USBRxBitDepth = 16;
		}
		else if (strcmp(argv[iDecode], "24") == 0)
		{
			p_audioTxConfigParam->bitPerSample = 32; // still 32 because the container is 32
			p_audioRxConfigParam->bitPerSample = 32; // still 32 because the container is 32
			g_appHandle.usbTxRx_handle.USBRxBitDepth = 24;
		}
		else if (strcmp(argv[iDecode], "32") == 0)
		{
			p_audioTxConfigParam->bitPerSample = 32;
			p_audioRxConfigParam->bitPerSample = 32;
			g_appHandle.usbTxRx_handle.USBRxBitDepth = 24; // 32 bits not supported by UAC 2.0 ....
		}
		else
		{
			PRINTF("FAIL: %s is not a valid choice\r\n",argv[iDecode]);
			return kStatus_SHELL_Error;
		}
	}

	/*
	 * set Tx source info
	 */
	iDecode = iDecode + 1;
	if (argc > iDecode)
	{
		if (strcmp(argv[iDecode], "1dmic") == 0)
		{
			g_appHandle.audioDefinition.audioTxPath_handle.operatingMode  = AUDIO_OM_ENABLE;
			p_audioWorkFlowHandle -> processEventPing 				     |=  AUDIO_TX_PDM_PING_BUFF;   	       // PDM mics ping condition
			p_audioWorkFlowHandle -> processEventPong 				     |=  AUDIO_TX_PDM_PONG_BUFF;      	   // PDM mics pong condition
			p_audioTxConfigParam  -> source 							  = AUDIO_SRC_OMNI_1DMIC;
			p_audioTxConfigParam  -> bufferHandle.p_pingBufferAddress[0]  = (void*) (g_sourceRawBuffPingTx);   // ping address for all dmic microphones
			p_audioTxConfigParam  -> bufferHandle.p_pongBufferAddress[0]  = (void*) (g_sourceRawBuffPongTx);   // pong address for all dmic microphones
			p_audioTxConfigParam  -> channelNumber				          = 1;                                 // 1 channel
		}
		else if (strcmp(argv[iDecode], "2dmic") == 0)
		{
			g_appHandle.audioDefinition.audioTxPath_handle.operatingMode  = AUDIO_OM_ENABLE;
			p_audioWorkFlowHandle -> processEventPing 				     |=  AUDIO_TX_PDM_PING_BUFF;   	       // PDM mics ping condition
			p_audioWorkFlowHandle -> processEventPong 				     |=  AUDIO_TX_PDM_PONG_BUFF;      	   // PDM mics pong condition
			p_audioTxConfigParam  -> source 							 = AUDIO_SRC_LINEAR_2DMIC;
			p_audioTxConfigParam  -> bufferHandle.p_pingBufferAddress[0] = (void*) (g_sourceRawBuffPingTx);    // ping address for all dmic microphones
			p_audioTxConfigParam  -> bufferHandle.p_pongBufferAddress[0] = (void*) (g_sourceRawBuffPongTx);    // pong address for all dmic microphones
			p_audioTxConfigParam  -> channelNumber				         = 2;                                  // 2 channels
		}
		else if (strcmp(argv[iDecode], "3dmic") == 0)
		{
			g_appHandle.audioDefinition.audioTxPath_handle.operatingMode  = AUDIO_OM_ENABLE;
			p_audioWorkFlowHandle -> processEventPing 				     |=  AUDIO_TX_PDM_PING_BUFF;   	       // PDM mics ping condition
			p_audioWorkFlowHandle -> processEventPong 				     |=  AUDIO_TX_PDM_PONG_BUFF;      	   // PDM mics pong condition
			p_audioTxConfigParam  -> source 							  = AUDIO_SRC_TRIANGULAR_3DMIC;
			p_audioTxConfigParam  -> bufferHandle.p_pingBufferAddress[0]  = (void*) (g_sourceRawBuffPingTx);   // ping address for all dmic microphones
			p_audioTxConfigParam  -> bufferHandle.p_pongBufferAddress[0]  = (void*) (g_sourceRawBuffPongTx);   // pong address for all dmic microphones
			p_audioTxConfigParam  -> channelNumber				          = 3;                                 // 3 channels
		}
#if (APP_PLATFORM == APP_PL_RT1170EVK)
		else if (strcmp(argv[iDecode], "4dmic") == 0)
		{
			g_appHandle.audioDefinition.audioTxPath_handle.operatingMode  = AUDIO_OM_ENABLE;
			p_audioWorkFlowHandle -> processEventPing 				     |=  AUDIO_TX_PDM_PING_BUFF;   	       // PDM mics ping condition
			p_audioWorkFlowHandle -> processEventPong 				     |=  AUDIO_TX_PDM_PONG_BUFF;      	   // PDM mics pong condition
			p_audioTxConfigParam  -> source 							  = AUDIO_SRC_SQUARE_TRILLIUM_4DMIC;
			p_audioTxConfigParam  -> bufferHandle.p_pingBufferAddress[0]  = (void*) (g_sourceRawBuffPingTx);   // ping address for all dmic microphones
			p_audioTxConfigParam  -> bufferHandle.p_pongBufferAddress[0]  = (void*) (g_sourceRawBuffPongTx);   // pong address for all dmic microphones
			p_audioTxConfigParam  -> channelNumber				          = 4;                                 // 4 channels
		}
#endif
		else if (strcmp(argv[iDecode], "no") == 0)
		{
			g_appHandle.audioDefinition.audioTxPath_handle.operatingMode = AUDIO_OM_DISABLE;

			p_audioTxConfigParam->source = AUDIO_SRC_NO;
		}
		else
		{
			PRINTF("FAIL: %s is not a valid choice\r\n",argv[iDecode]);
			return kStatus_SHELL_Error;
		}

		/*
		 *  Compute byte per frame
		 */
		p_audioTxConfigParam->bytePerFramePerChannel = p_audioTxConfigParam->samplePerFrame         * p_audioTxConfigParam->bitPerSample    / 8;
		p_audioTxConfigParam->bytePerFrame 			 = p_audioTxConfigParam->bytePerFramePerChannel * p_audioTxConfigParam->channelNumber;
		//check byte per frame is 4 bytes align
		if (  ((p_audioTxConfigParam->bytePerFramePerChannel %4)!= 0)
			&&((p_audioTxConfigParam->bytePerFrame           %4)!= 0))
			{
				PRINTF("FAIL: BytePerFrame not 4-bytes align\r\n\n");
				return kStatus_SHELL_Error;
			}
	}

	/*
	 * set Tx sink info
	 */
	iDecode = iDecode + 1;
	if (argc > iDecode)
	{
		g_appHandle.usbTxRx_handle.USBTxchannelNumber = 1; 											 	// mandatory because usb may be enable by the TX

		if (strcmp(argv[iDecode], "usb1") == 0)
		{
			p_audioTxConfigParam->sink 					  = AUDIO_SINK_USB1;							// Tx output is USB
			g_appHandle.usbTxRx_handle.USBTxchannelNumber = 1; 											// 1 USB channel Tx
		}
		else if (strcmp(argv[iDecode], "usb4") == 0)
		{
			p_audioTxConfigParam->sink 					  = AUDIO_SINK_USB4;							// Tx output is USB
			g_appHandle.usbTxRx_handle.USBTxchannelNumber = 4; 											// 4 USB channel Tx
		}
		else if (strcmp(argv[iDecode], "usb5") == 0)
		{
			p_audioTxConfigParam->sink 					  = AUDIO_SINK_USB5;							// Tx output is USB
			g_appHandle.usbTxRx_handle.USBTxchannelNumber = 5; 											// 5 USB channel Tx
		}
		else if (strcmp(argv[iDecode], "no") == 0)
		{
			p_audioTxConfigParam->sink   				  = AUDIO_SINK_NO;
		}
		else
		{
			PRINTF("FAIL: %s is not a valid choice\r\n",argv[iDecode]);
			return kStatus_SHELL_Error;
		}

		// Check compatibility
		if (g_appHandle.usbTxRx_handle.USBTxchannelNumber > APP_USB_TX_CHANNEL_MAX)
		{
			PRINTF("FAIL: %s is not a valid choice (maximum USB TX channel number is set to %d)\r\n",argv[iDecode],APP_USB_TX_CHANNEL_MAX);
			return kStatus_SHELL_Error;
		}
	}

	/*
	 * set Rx source info
	 */
	iDecode = iDecode + 1;
	if (argc > iDecode)
	{
		g_appHandle.usbTxRx_handle.USBRxchannelNumber = 2; 											 	// mandatory because usb may be enable by the TX
		if (strcmp(argv[iDecode], "usb1") == 0)
		{
			p_audioRxConfigParam->source 				  = AUDIO_SRC_USB1;								 // Rx source is USB
			g_appHandle.usbTxRx_handle.USBRxchannelNumber = 1; 											 // 1 USB channel Tx
		}
		else if (strcmp(argv[iDecode], "usb2") == 0)
		{
			p_audioRxConfigParam->source 				  = AUDIO_SRC_USB2;								 // Rx source is USB
			g_appHandle.usbTxRx_handle.USBRxchannelNumber = 2; 											 // 2 USB channel Tx
		}
		else if (strcmp(argv[iDecode], "tx0") == 0)
		{
			p_audioRxConfigParam->source 				  = AUDIO_SRC_TX0;						 		 // Rx source is Tx channel 1
		}
		else if (strcmp(argv[iDecode], "tx0_1") == 0)
		{
			p_audioRxConfigParam->source 				  = AUDIO_SRC_TX0_1;						 	 // Rx source is Tx channel 1/2
		}
		// TODO add tx2_3
		else if (strcmp(argv[iDecode], "no") == 0)
		{
			p_audioRxConfigParam->source = AUDIO_SRC_NO;
		}
		else
		{
			PRINTF("FAIL: %s is not a valid choice\r\n",argv[iDecode]);
			return kStatus_SHELL_Error;
		}

		// Check compatibility
		if (g_appHandle.usbTxRx_handle.USBRxchannelNumber > APP_USB_RX_CHANNEL_MAX)
		{
			PRINTF("FAIL: %s is not a valid choice (maximum USB RX channel number is set to %d)\r\n",argv[iDecode],APP_USB_RX_CHANNEL_MAX);
			return kStatus_SHELL_Error;
		}

	}

	/*
	 * set Rx sink info
	 */
	iDecode = iDecode + 1;
	if (argc > iDecode)
	{
		if (strcmp(argv[iDecode], "1spk") == 0)
		{
			g_appHandle.audioDefinition.audioRxPath_handle.operatingMode  = AUDIO_OM_ENABLE;
			p_audioWorkFlowHandle -> processEventPing 				     |=  AUDIO_TX_SAI0_PING_BUFF;   	    // SAI Tx ping condition
			p_audioWorkFlowHandle -> processEventPong 				     |=  AUDIO_TX_SAI0_PONG_BUFF;      	    // SAI Tx pong condition
			p_audioRxConfigParam   -> bufferHandle.p_pingBufferAddress[0] = (void*) (g_sinkBuffPingRx);  		// ping address for all SAI output
			p_audioRxConfigParam   -> bufferHandle.p_pongBufferAddress[0] = (void*) (g_sinkBuffPongRx);  		// pong address for all SAI output
			p_audioRxConfigParam   -> channelNumber				          = 2;                           		// force to 2 channels for I2S audio codec compatibility. 2 channels are required because audio codec is using I2S and I2S is always 2 channels.
			p_audioRxConfigParam   -> sink 								  = AUDIO_SINK_SPEAKER1;		 		// Rx sink is speaker mono
		}
		else if (strcmp(argv[iDecode], "1spkdiff") == 0)
		{
			g_appHandle.audioDefinition.audioRxPath_handle.operatingMode  = AUDIO_OM_ENABLE;
			p_audioWorkFlowHandle -> processEventPing 				     |=  AUDIO_TX_SAI0_PING_BUFF;   	    // SAI Tx ping condition
			p_audioWorkFlowHandle -> processEventPong 				     |=  AUDIO_TX_SAI0_PONG_BUFF;      	    // SAI Tx pong condition
			p_audioRxConfigParam   -> bufferHandle.p_pingBufferAddress[0] = (void*) (g_sinkBuffPingRx);  		// ping address for all SAI output
			p_audioRxConfigParam   -> bufferHandle.p_pongBufferAddress[0] = (void*) (g_sinkBuffPongRx);  		// pong address for all SAI output
			p_audioRxConfigParam   -> channelNumber				          = 2;                           		// 2 channels
			p_audioRxConfigParam   -> sink 								  = AUDIO_SINK_SPEAKER1DIFF;		 	// Rx sink is speaker mono
		}
		else if (strcmp(argv[iDecode], "2spk") == 0)
		{
			g_appHandle.audioDefinition.audioRxPath_handle.operatingMode  = AUDIO_OM_ENABLE;
			p_audioWorkFlowHandle -> processEventPing 				     |=  AUDIO_TX_SAI0_PING_BUFF;   	    // SAI Tx ping condition
			p_audioWorkFlowHandle -> processEventPong 				     |=  AUDIO_TX_SAI0_PONG_BUFF;      	    // SAI Tx pong condition
			p_audioRxConfigParam   -> bufferHandle.p_pingBufferAddress[0] = (void*) (g_sinkBuffPingRx);  		// ping address for all output
			p_audioRxConfigParam   -> bufferHandle.p_pongBufferAddress[0] = (void*) (g_sinkBuffPongRx);  		// pong address for all output
			p_audioRxConfigParam   -> channelNumber				          = 2;                           		// 2 channel
			p_audioRxConfigParam   -> sink 								  = AUDIO_SINK_SPEAKER2;			 	 // Rx sink is speaker mono
		}
		else if (strcmp(argv[iDecode], "no") == 0)
		{
			g_appHandle.audioDefinition.audioRxPath_handle.operatingMode  = AUDIO_OM_DISABLE;
			p_audioRxConfigParam->sink   								  = AUDIO_SINK_NO;
		}
		else
		{
			PRINTF("FAIL: %s is not a valid choice\r\n",argv[iDecode]);
			return kStatus_SHELL_Error;
		}

		// Compute byte per frame
		p_audioRxConfigParam->bytePerFramePerChannel = p_audioRxConfigParam->samplePerFrame         * p_audioRxConfigParam->bitPerSample   / 8;
		p_audioRxConfigParam->bytePerFrame 			 = p_audioRxConfigParam->bytePerFramePerChannel * p_audioRxConfigParam->channelNumber;
	}

	/*
	 * Check Tx & Rx path parameter are compatible together according platform selected
	 */
	retStatusFunc = initCheckDefinitionPathTxRx( &p_audioDefinition_handle->audioTxPath_handle,
											     &p_audioDefinition_handle->audioRxPath_handle);
	if (retStatusFunc == kStatus_Success)
	{
		// do nothing
	}
	else if (retStatusFunc == kStatus_NullPointer)
	{
		PRINTF("FAIL: Tx-Rx path initialization error : Null pointer\r\n");
		return kStatus_SHELL_Error;
	}
	else if (retStatusFunc == kStatus_OutOfRange)
	{
		PRINTF("FAIL: Tx-Rx path initialization error: Value out of range\r\n");
		return kStatus_SHELL_Error;
	}
	else if (retStatusFunc == kStatus_NotCompatible)
	{
		PRINTF("FAIL: Tx-Rx path initialization error: Not compatible configuration 1 \r\n");
		return kStatus_SHELL_Error;
	}
	else
	{
		PRINTF("FAIL: Tx-Rx path initialization error: Unknown error\r\n");
		return kStatus_SHELL_Error;
	}

	/*
	 * set process
	 *     set process parameters
	 *     check parameters are compatible with process
	 */
	iDecode  = iDecode + 1; // to decode the command
	iProcess = 0;           // to run over process list
	while(argc > iDecode)
	{
		if (strcmp(argv[iDecode], "no") == 0)
		{
			g_appHandle.audioDefinition.processList[iProcess] = AUDIO_PROC_NO;
		}
		/* CONVERSA */
		else if (
				/* SOFTWARE PACK CONFIGURATION */
					(strcmp(argv[iDecode], "conversaswp16k") == 0)        // else if conversa process is in the list
				 || (strcmp(argv[iDecode], "conversaswp32k") == 0)
				/* end OFTWARE PACK CONFIGURATION */
				)
		{
#ifdef CONVERSA_PRESENT
			/*
			 * Select conversa parameters file
			 *     Select the conversa parameter file according the command
			 *     Conversa parameter file will be load during conversa init function
			 *     check conversa parameter file
			 */
			g_appHandle.audioDefinition.swIpConversa_handle.conversaTuningParamStruct.isThereTuningFile = PL_FALSE; // by default no configuration file available

			/* SOFTWARE PACK CONFIGURATION */
			if (strcmp(argv[iDecode], "conversaswp16k") == 0)
			{
				PRINTF("Conversa parameter file\n\r\t -Name: conversa_parameter_config_RT1170swp16k\r\n");
				g_appHandle.audioDefinition.swIpConversa_handle.conversaTuningParamStruct.p_conversaTuningParameter = (conversa_parameter_config_t*) &conversa_parameter_config_RT1170swp16k;
				g_appHandle.audioDefinition.swIpConversa_handle.conversaTuningParamStruct.isThereTuningFile  = PL_TRUE;
			}
			else if (strcmp(argv[iDecode], "conversaswp32k") == 0)
			{
				PRINTF("Conversa parameter file\n\r\t -Name: conversa_parameter_config_RT1170swp32k\r\n");
				g_appHandle.audioDefinition.swIpConversa_handle.conversaTuningParamStruct.p_conversaTuningParameter = (conversa_parameter_config_t*) &conversa_parameter_config_RT1170swp32k;
				g_appHandle.audioDefinition.swIpConversa_handle.conversaTuningParamStruct.isThereTuningFile   = PL_TRUE;
			}
			/* end SOFTWARE PACK CONFIGURATION */

			else
			{
				PRINTF("FAIL: %s is not a valid choice\r\n",argv[iDecode]);
				return kStatus_SHELL_Error;
			}

			/*
			 *  Check Conversa parameter file and library compatibility
			 */
			// Is there a tuning file?
			if (g_appHandle.audioDefinition.swIpConversa_handle.conversaTuningParamStruct.isThereTuningFile == PL_TRUE)
			{
				PRINTF("\t -Version: %d.%d.%d\r\n",g_appHandle.audioDefinition.swIpConversa_handle.conversaTuningParamStruct.p_conversaTuningParameter->conversa_lib_version[0],
                                                   g_appHandle.audioDefinition.swIpConversa_handle.conversaTuningParamStruct.p_conversaTuningParameter->conversa_lib_version[1],
												   g_appHandle.audioDefinition.swIpConversa_handle.conversaTuningParamStruct.p_conversaTuningParameter->conversa_lib_version[2]);
				PRINTF("\t -Information: %s\r\n",  g_appHandle.audioDefinition.swIpConversa_handle.conversaTuningParamStruct.p_conversaTuningParameter->info_str);

				// check parameter file compatibility with library
				retStatusFunc = initCheckDefinitionParameterFileConversa( &g_appHandle.audioDefinition.swIpConversa_handle );
				if (retStatusFunc != kStatus_Success)
				{
					PRINTF("FAIL: Conversa parameter files & Conversa library constant are not compatible\r\n");
					return kStatus_SHELL_Error;
				}
				else
				{
					PRINTF("\t -Check: Compatible with current Conversa library constant\r\n");
				}
			}
			else
			{
				PRINTF("WARNING: No tuning file present for %s choice. Keep the default Conversa parameters\r\n",argv[iDecode]);
			}

			/*
			 *  Check if Conversa is compatible with Tx\Rx path parameters
			 */
			retStatusFunc = initCheckDefinitionPathTxRxConversa( &g_appHandle.audioDefinition.audioTxPath_handle,
																 &g_appHandle.audioDefinition.audioRxPath_handle,
																 &g_appHandle.audioDefinition.swIpConversa_handle
															   );
			if (retStatusFunc != kStatus_Success)
			{
				PRINTF("FAIL: Conversa & Tx-Rx path configuration are not compatible\r\n");
				return kStatus_SHELL_Error;
			}
			else
			{
				// Conversa tx path is enabled
				g_appHandle.audioDefinition.processList[iProcess]	= AUDIO_PROC_CONVERSA_TXRX;  			// add conversa TX/RX process in the process list
				iProcess = iProcess + 1; 		// increment process list
			}
#else
			PRINTF("FAIL: conversa is not present\r\n");
			return kStatus_SHELL_Error;
#endif // end if CONVERSA
		}
		/* LOAD SIMULATION PROCESS */
		else if (strcmp(argv[iDecode], "mips5") == 0)				// If simulate mips load x %
		{
			g_appHandle.audioDefinition.processList[iProcess] = AUDIO_PROC_MIPS_5;
			iProcess = iProcess + 1; 		// increment process list
		}
		else if (strcmp(argv[iDecode], "mips25") == 0)				// If simulate mips load x %
		{
			g_appHandle.audioDefinition.processList[iProcess] = AUDIO_PROC_MIPS_25;
			iProcess = iProcess + 1; 		// increment process list
		}
		else if (strcmp(argv[iDecode], "mips50") == 0)				// If simulate mips load x %
		{
			g_appHandle.audioDefinition.processList[iProcess] = AUDIO_PROC_MIPS_50;
			iProcess = iProcess + 1; 		// increment process list
		}
		else if (strcmp(argv[iDecode], "mips75") == 0)				// If simulate mips load x %
		{
			g_appHandle.audioDefinition.processList[iProcess] = AUDIO_PROC_MIPS_75;
			iProcess = iProcess + 1; 		// increment process list
		}
		else if (strcmp(argv[iDecode], "mips95") == 0)				// If simulate mips load x %
		{
			g_appHandle.audioDefinition.processList[iProcess] = AUDIO_PROC_MIPS_95;
			iProcess = iProcess + 1; 		// increment process list
		}
		else
		{
			PRINTF("FAIL: %s is not a valid choice\r\n",argv[iDecode]);
			return kStatus_SHELL_Error;
		}
		iDecode  = iDecode + 1;  // increment decode list
	}


	/******************************************************************
	 * SW Process init use case
	 * 		- Cross check each software process with the Tx and Rx path
	 */
	if (shellRetStatus == kStatus_SHELL_Success)
	{
		retStatusFunc = initAudioSoftwareProcess( p_audioDefinition_handle );
		if (retStatusFunc != kStatus_Success)
		{
			PRINTF("FAIL: Fail to init software Process\r\n");
			return kStatus_SHELL_Error;
		}
	}
	PRINTF("\r\n"); // for shell print


	/******************************************************************
	 * Audio hardware init use case
	 *
	 */
	if (shellRetStatus == kStatus_SHELL_Success)
	{
		retStatusFunc = initAudioUseCase( p_audioDefinition_handle );
		if (retStatusFunc != kStatus_Success)
		{
			PRINTF("FAIL: Fail to init the hardware\r\n");
			return kStatus_SHELL_Error;
		}
	}
	PRINTF("\r\n"); // for shell print


	/********************************************
	 * Create audio task to run the audio use case
	 */
	if (shellRetStatus == kStatus_SHELL_Success)
	{
		/* Create Task Tx and Rx */
		retStatusFunc = AUDIO_createTask ( p_audioDefinition_handle );
		if (retStatusFunc != kStatus_Success)
		{
			PRINTF("FAIL: Fail to create Tx / Rx task\r\n");
			return kStatus_SHELL_Error;
		}
	}

	return kStatus_SHELL_Success;
}

/*******************/
/* STATIC FUNCTION */
/*******************/
void loopBackModeSecurityLoop()
{
	loopBackConfigSecurityStep = PL_TRUE;   // unlock the loop back security
	PRINTF("\n\nWARNING : FOR LOOPBACK CONFIGURATION REMOVE SPEAKER JACK CONNECTOR AND PLUG A HEADSET TO AVOID HOWL AND POTENTIAL SPEAKER DAMAGE: WARNING\r\n\n");
	PRINTF("Type again the command to launch loop back configuration\r\n");
	return;
}

#endif // (APP_PLATFORM == APP_PL_RT1170EVK)
/*${function:end}*/
