/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

//Standard headers
#include <stdint.h>
#include <ctype.h>

//platform headers

//fsl driver headers
#include "fsl_debug_console.h"
#include "fsl_shell.h"

// Application headers
#include "appGlobal.h"
#include "commandShell_LPC55S69_CFG1.h"
// Multi core manager
#include "Core1MCMGR.h"
#include "mcmgr.h"
#ifdef CONVERSA_PRESENT
	#include "conversa_parameter_config_LPC55S69swp16k.src"
#endif

#if (APP_PLATFORM == APP_PL_LPC55S69EVK)

/*******************************************************************************
 * Definitions
 ******************************************************************************/
static void loopBackModeSecurityLoop();
/*******************************************************************************
 * Variables
 ******************************************************************************/
extern volatile T_CommonVarSharedByCore0AndCore1 	CommonVarSharedByCore0AndCore1;		   // shared structure between core0 & core1
extern volatile PL_BOOL						        g_ConversaIsInit;
PL_INT8                                             loopBackConfigSecurityStep = PL_FALSE; // to secure the user hit 2 times the loopback command and read the warning message

/*******************************************************************************
 * Code
 ******************************************************************************/
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
	volatile PL_INT16        iProcess 	                    = 0;    // counter for process list
	volatile PL_INT16        iDecode  	                    = 0; 	// counter for decoder

	// Others
	PL_INT32                 temp1_32b 		                = 0; 	// temporary 32 bit value
	PL_INT32                 temp2_32b 		                = 0; 	// temporary 32 bit value

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

//here needs to prevent from executing a new voicecall command if it is already in voicecall

if(g_appHandle.audioDefinition.audioPathIsInit ==1)
{
	PRINTF("FAIL: Audio configuration is already running, please type 'reset' command before select a new configuration\r\n\n");
	return kStatus_SHELL_Error;
}

	/*
	 * Common configuration
	 */

	/*
	 * set sample rate or default configuration
	 */
	iDecode = 1;
	if (argc > iDecode)
	{
		/* begin SOFTWARE PACK CONFIGURATION */
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
				strcpy(&cmdDefault[0][0],  "lpc55s69SWPmockup");    		// platform and materials
				strcpy(&cmdDefault[1][0],  "16000");            // frame rate in Hz
				strcpy(&cmdDefault[2][0],  "64");				// sample per frame
				strcpy(&cmdDefault[3][0],  "32");				// bit per sample
				strcpy(&cmdDefault[4][0],  "2mic");	            // Tx source
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
			PRINTF("\nUse case: Usb microphones and Usb speaker. Fs=16kHz\r\n");
			argc = 9;
			strcpy(&cmdDefault[0][0],  "lpc55s69SWPmockup");  		// platform and materials
			strcpy(&cmdDefault[1][0],  "16000");            // frame rate in Hz
			strcpy(&cmdDefault[2][0],  "64");				// sample per frame
			strcpy(&cmdDefault[3][0],  "32");				// bit per sample
			strcpy(&cmdDefault[4][0],  "2mic");	            // Tx source
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
		else if (strcmp(argv[iDecode], "spswp16k") == 0)
		{
			PRINTF("\n  Use case: Speaker phone with Conversa software (required voice call LP55S69 mockup). Teams conferencing device 1.5m Fs=16kHz.\r\n");
			argc = 9;
			strcpy(&cmdDefault[0][0],  "lpc55s69SWPmockup");// platform and materials
			strcpy(&cmdDefault[1][0],  "16000");            // frame rate in Hz
			strcpy(&cmdDefault[2][0],  "64");				// sample per frame
			strcpy(&cmdDefault[3][0],  "32");				// bit per sample
			strcpy(&cmdDefault[4][0],  "2mic");	            // Tx source
			strcpy(&cmdDefault[5][0],  "usb4");				// Tx sink
			strcpy(&cmdDefault[6][0],  "usb1");				// Rx source
			strcpy(&cmdDefault[7][0],  "1spk");			    // Rx sink
			strcpy(&cmdDefault[8][0],  "conversaswp16k");   // process
			// print the in line command
			PRINTF("( ");
			for (iDecode=0 ; iDecode < argc ; iDecode++)
			{
				argv[iDecode] = &cmdDefault[iDecode][0];
				PRINTF("%s ", argv[iDecode]);
			}
			PRINTF(")\r\n\n");
			iDecode = 0; // set iDecode to 0 for the following decoding command
		}
		/* end SOFTWARE PACK CONFIGURATION */
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
			if (strcmp(argv[iDecode], "lpc55s69") == 0)
			{
				g_appHandle.platformAndMaterials = APP_PLATFORM_MATERIALS_LPC55S69;
			}
/* begin SOFTWARE PACK CONFIGURATION */
			else if (strcmp(argv[iDecode], "lpc55s69SWPmockup") == 0)
			{
				g_appHandle.platformAndMaterials = APP_PLATFORM_MATERIALS_LPC55S69_TB2136_RDSPMICARRAY;
			}
/* end SOFTWARE PACK CONFIGURATION */
/* CUSTOMER CONFIGURATION */
			else if (strcmp(argv[iDecode], "LPC55S69CUSTOME") == 0)
			{
				g_appHandle.platformAndMaterials = APP_PLATFORM_MATERIALS_LPC55S69_CUSTOMER;
			}
/* end CUSTOMER CONFIGURATION */
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
		else if (strcmp(argv[iDecode], "32000") == 0)
		{
			p_audioTxConfigParam->sampleRate = 32000;
			p_audioRxConfigParam->sampleRate = 32000;
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
		if (temp1_32b % 8)
		{
			PRINTF("FAIL: %s is not a valid choice ( not a multiple of 8 )\r\n",argv[iDecode]);
			return kStatus_SHELL_Error;
		}
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
		if (strcmp(argv[iDecode], "32") == 0)
		{
			p_audioTxConfigParam->bitPerSample = 32;
			p_audioRxConfigParam->bitPerSample = 32;
			g_appHandle.UsbCompositeDev_handle.AUDIO_OUT_FORMAT_SIZE =4;
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
		if (strcmp(argv[iDecode], "1mic") == 0)
		{
			p_audioTxConfigParam  -> channelNumber				          = 2;                                 // 1 channel but I2S always stereo
			p_audioTxConfigParam  -> source 							  = AUDIO_SRC_OMNI_1DMIC;
			p_audioWorkFlowHandle -> processEventPing 				      =  AUDIO_WF_PING;   	               // mics ping condition
			p_audioWorkFlowHandle -> processEventPong 				      =  AUDIO_WF_PONG;      	           //  mics pong condition
			p_audioTxConfigParam  -> bufferHandle.p_pingBufferAddress[0]  = (void*) (g_sourceRawBuffPingTx);   // ping address for all I2S microphones
			p_audioTxConfigParam  -> bufferHandle.p_pongBufferAddress[0]  = (void*) (g_sourceRawBuffPongTx);   // pong address for all I2S microphones
		}else if (strcmp(argv[iDecode], "2mic") == 0)
		{
			p_audioTxConfigParam  -> channelNumber				          = 2;                                 // 2 channel
			p_audioTxConfigParam  -> source 							  = AUDIO_SRC_LINEAR_2DMIC;
			p_audioWorkFlowHandle -> processEventPing 				      =  AUDIO_WF_PING;   	               // mics ping condition
			p_audioWorkFlowHandle -> processEventPong 				      =  AUDIO_WF_PONG;      	           // mics pong condition
			p_audioTxConfigParam  -> bufferHandle.p_pingBufferAddress[0]  = (void*) (g_sourceRawBuffPingTx);   // ping address for all I2S microphones
			p_audioTxConfigParam  -> bufferHandle.p_pongBufferAddress[0]  = (void*) (g_sourceRawBuffPongTx);   // pong address for all I2S microphones
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
		g_appHandle.UsbCompositeDev_handle.AUDIO_IN_FORMAT_CHANNELS = 1; 							 	// mandatory because usb may be enable by the TX

		if (strcmp(argv[iDecode], "usb1") == 0)
		{
			p_audioTxConfigParam->sink 					  = AUDIO_SINK_USB1;							// Tx output is USB
			g_appHandle.UsbCompositeDev_handle.AUDIO_IN_FORMAT_CHANNELS = 1;							// 1 USB channel Tx
		}
		else if (strcmp(argv[iDecode], "usb4") == 0)
		{
			p_audioTxConfigParam->sink 					  = AUDIO_SINK_USB4;							// Tx output is USB
			g_appHandle.UsbCompositeDev_handle.AUDIO_IN_FORMAT_CHANNELS = 4;							// 4 USB channel Tx
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
		if (g_appHandle.UsbCompositeDev_handle.AUDIO_IN_FORMAT_CHANNELS > APP_USB_TX_CHANNEL_MAX)
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
		g_appHandle.UsbCompositeDev_handle.AUDIO_OUT_FORMAT_CHANNELS = 2; 							 	// mandatory because usb may be enable by the TX
		if (strcmp(argv[iDecode], "usb1") == 0)
		{
			p_audioRxConfigParam->source 				  = AUDIO_SRC_USB1;								 // Rx source is USB
			g_appHandle.UsbCompositeDev_handle.AUDIO_OUT_FORMAT_CHANNELS = 1; 							 // 1 USB channel Tx
		}
		else if (strcmp(argv[iDecode], "usb2") == 0)
		{
			p_audioRxConfigParam->source 				  = AUDIO_SRC_USB2;								 // Rx source is USB
			g_appHandle.UsbCompositeDev_handle.AUDIO_OUT_FORMAT_CHANNELS = 2; 							 // 2 USB channel Tx
		}
		else if (strcmp(argv[iDecode], "tx0") == 0)
		{
			p_audioRxConfigParam->source 				  = AUDIO_SRC_TX0;						 		 // Rx source is Tx channel 1
		}
		else if (strcmp(argv[iDecode], "tx0_1") == 0)
		{
			p_audioRxConfigParam->source 				  = AUDIO_SRC_TX0_1;						 	 // Rx source is Tx channel 1/2
		}
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
		if (g_appHandle.UsbCompositeDev_handle.AUDIO_OUT_FORMAT_CHANNELS > APP_USB_RX_CHANNEL_MAX)
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
			p_audioRxConfigParam   -> channelNumber				          = 2;                           		// force to 2 channels for I2S audio codec compatibility. 2 channels are required because audio codec is using I2S and I2S is always 2 channels.
			p_audioRxConfigParam   -> sink 								  = AUDIO_SINK_SPEAKER1;		 		// Rx sink is speaker mono
			p_audioWorkFlowHandle -> processEventPing 				      =  AUDIO_WF_PING;   	               // PDM mics ping condition
			p_audioWorkFlowHandle -> processEventPong 				      =  AUDIO_WF_PONG;      	           // PDM mics pong condition
#if RX_PATH_PRESENT
			p_audioRxConfigParam   -> bufferHandle.p_pingBufferAddress[0] = (void*) (g_sinkBuffPingRx);  		// ping address for all output
			p_audioRxConfigParam   -> bufferHandle.p_pongBufferAddress[0] = (void*) (g_sinkBuffPongRx);  		// pong address for all output
#endif
		}
		else if (strcmp(argv[iDecode], "2spk") == 0)
		{
			g_appHandle.audioDefinition.audioRxPath_handle.operatingMode  = AUDIO_OM_ENABLE;
			p_audioRxConfigParam   -> channelNumber				          = 2;                           		// force to 2 channels for I2S audio codec compatibility. 2 channels are required because audio codec is using I2S and I2S is always 2 channels.
			p_audioRxConfigParam   -> sink 								  = AUDIO_SINK_SPEAKER2;		 		// Rx sink is speaker mono
			p_audioWorkFlowHandle -> processEventPing 				      =  AUDIO_WF_PING;   	                // PDM mics ping condition
			p_audioWorkFlowHandle -> processEventPong 		  			  =  AUDIO_WF_PONG;      	            // PDM mics pong condition
#if RX_PATH_PRESENT
			p_audioRxConfigParam   -> bufferHandle.p_pingBufferAddress[0] = (void*) (g_sinkBuffPingRx);  		// ping address for all output
			p_audioRxConfigParam   -> bufferHandle.p_pongBufferAddress[0] = (void*) (g_sinkBuffPongRx);  		// pong address for all output
#endif
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
	if (retStatusFunc == kStatus_NoData)
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
		PRINTF("FAIL: Tx-Rx path initialization error: Not compatible configuration\r\n");
		return kStatus_SHELL_Error;
	}
	else if (retStatusFunc == kStatus_Success)
	{
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
				/* begin SOFTWARE PACK CONFIGURATION */
					(strcmp(argv[iDecode], "conversaswp16k") == 0)        // else if conversa process is in the list
				||  (strcmp(argv[iDecode], "conversaML_NOAEC_swp16k") == 0)
				/* end SOFTWARE PACK CONFIGURATION */
				)
		{
#ifdef CONVERSA_PRESENT
			/*
			 * Select conversa parameters file
			 *     Will be load during cconversa init function
			 */
			g_appHandle.audioDefinition.ConfigWithConversa = PL_TRUE;
			g_appHandle.audioDefinition.swIpConversa_handle.conversaTuningParamStruct.isThereTuningFile = PL_FALSE; // by default no configuration file available
			/* begin SOFTWARE PACK CONFIGURATION */
			if (strcmp(argv[iDecode], "conversaswp16k") == 0)
			{
				PRINTF("Conversa parameter file\n\r\t -Name: conversa_parameter_config_LPC55S69swp16k\r\n");
				g_appHandle.audioDefinition.swIpConversa_handle.conversaTuningParamStruct.p_conversaTuningParameter = (conversa_parameter_config_t*) &conversa_parameter_config_LPC55S69swp16k;
				g_appHandle.audioDefinition.swIpConversa_handle.conversaTuningParamStruct.isThereTuningFile  = PL_TRUE;
			}
			/* end SOFTWARE PACK CONFIGURATION */
			else if (strcmp(argv[iDecode], "conversaML_NOAEC_swp16k") == 0)        // else if conversa process is in the list
			{
			}
			else
			{
				PRINTF("FAIL: %s is not a valid choice\r\n",argv[iDecode]);
				return kStatus_SHELL_Error;
			}
			// Conversa tx path is enabled
			g_appHandle.audioDefinition.processList[iProcess]	= AUDIO_PROC_CONVERSA_TXRX;  			// add conversa TX/RX process in the process list
			iProcess = iProcess + 1; 		// increment process list
#else
			PRINTF("FAIL: conversa is not present\r\n");
			return kStatus_SHELL_Error;
#endif// Is there a tuning file?
			if (g_appHandle.audioDefinition.swIpConversa_handle.conversaTuningParamStruct.isThereTuningFile == PL_TRUE)
			{

				PRINTF("\t -Version: %d.%d.%d\r\n",g_appHandle.audioDefinition.swIpConversa_handle.conversaTuningParamStruct.p_conversaTuningParameter->conversa_lib_version[0],
                                                   g_appHandle.audioDefinition.swIpConversa_handle.conversaTuningParamStruct.p_conversaTuningParameter->conversa_lib_version[1],
												   g_appHandle.audioDefinition.swIpConversa_handle.conversaTuningParamStruct.p_conversaTuningParameter->conversa_lib_version[2]);
				PRINTF("\t -Information: %s\r\n",  g_appHandle.audioDefinition.swIpConversa_handle.conversaTuningParamStruct.p_conversaTuningParameter->info_str);
			}

			else
			{
				PRINTF("WARNING: No tuning file present for %s choice. Keep the default Conversa parameters\r\n",argv[iDecode]);
			}
		}
		else
		{
			PRINTF("FAIL: %s is not a valid choice\r\n",argv[iDecode]);
			return kStatus_SHELL_Error;
		}
		iDecode  = iDecode + 1;  // increment decode list
	}
	/*
	 * Set audioPathIsDefined to TRUE
	 */
	g_appHandle.audioDefinition.audioPathIsDefined =PL_TRUE;
	return kStatus_SHELL_Success;
}

/*******************/
/* STATIC FUNCTION */
/*******************/
static void loopBackModeSecurityLoop()
{
	loopBackConfigSecurityStep = PL_TRUE;   // unlock the loop back security
	PRINTF("\n\nWARNING : FOR LOOPBACK CONFIGURATION REMOVE SPEAKER JACK CONNECTOR AND PLUG A HEADSET TO AVOID HOWL AND POTENTIAL SPEAKER DAMAGE: WARNING\r\n\n");
	PRINTF("Type again the command to launch loop back configuration\r\n");
	return;
}

#endif/*#if (APP_PLATFORM == APP_PL_LPC55S69EVK)*/
