/*
 * Copyright 2020-2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

//general
#include "PL_platformTypes.h"
#include "appGlobal.h"
#include "tools.h"

// memory section replacement
#include <cr_section_macros.h>

// Init
#include "init.h"

// Audio
#include "audio.h"
#include "audioTxRxTask.h"

// Sw Ip: Conversa
#include "swProcess.h"

// CMSIS lib
#include "arm_math.h"

// usb
#include "usb_audio_unified.h"

// Sw Ip: Conversa
#ifdef CONVERSA_PRESENT
  #include "RdspConversaPlugin.h"
  #include "RdspConversaPluginConfig.h"
#endif

// GPIO for debug and MIPS measure on J26
#ifdef MIPS_MEASURE_GPIO
	#include "fsl_gpio.h"
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define MIPS_DUMMY_FUNCTION_STACK_USAGE 2 // stack usage use in the mips dummy task => to be set to 2 when stack usage not tested

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
PL_FLOAT mipsDummyFunction(TickType_t tickNumber , PL_FLOAT value_flt);

/*******************************************************************************
 * Code
 ******************************************************************************/
__RAMFUNC(RAM) // force to put the code instruction of this function in RAM (FLEXRAMRAM_ITC)
void AUDIO_TxRxTask(void* param)
{
	// error handle
	status_t 	 			 retStatus 	 	 = kStatus_Success;
	status_t 	 			 retStatusFunc   = kStatus_Success;

	// Parameters
	AUDIO_definition_st*     		  p_audioDefinition_handle = (AUDIO_definition_st*) param;
	AUDIO_definitionPath_st* 		  p_audioTxPath_handle     = &p_audioDefinition_handle->audioTxPath_handle;
	AUDIO_definitionPath_st* 		  p_audioRxPath_handle	   = &p_audioDefinition_handle->audioRxPath_handle;
	volatile AUDIO_workFlowHandle_st* p_audioWorkFlow_handle   = &p_audioDefinition_handle->audioWorkFlow_handle;

#ifdef CONVERSA_PRESENT
	AUDIO_conversa_st*       p_swIpConversa_handle 		 = &p_audioDefinition_handle->swIpConversa_handle;
#endif

	// raw audio data buffer
	PL_INT8* 				 p_receiveRawBuff_Tx_8b; 		//  8 bit pointer to the raw buffer
	PL_INT8* 				 p_receiveRawBuff_Rx_8b; 		//  8 bit pointer to the raw buffer

	// According to Tx/Rx path bit with different buffer pointer format is used to address the sample
	// Tx process buffer pointer
	PL_INT8* 		  pp_processBuff_Tx_8b  [ APP_AUDIO_TX_CHANNEL_NUMBER_MAX ]; 		//  8 bit pointer to the TX process buffer
	PL_INT16*		  pp_processBuff_Tx_16b [ APP_AUDIO_TX_CHANNEL_NUMBER_MAX ];		// 16 bit pointer to the TX process buffer
	PL_INT32*		  pp_processBuff_Tx_32b [ APP_AUDIO_TX_CHANNEL_NUMBER_MAX ];		// 32 bit pointer to the TX process buffer

	// Rx reference output buffer pointer
	PL_INT8* 		  pp_processBuff_Rx_8b  [ APP_AUDIO_RX_CHANNEL_NUMBER_MAX ];		//  8 bit pointer to the Rx output reference buffer
	PL_INT16*		  pp_processBuff_Rx_16b [ APP_AUDIO_RX_CHANNEL_NUMBER_MAX ];    	// 16 bit pointer to the Rx output reference buffer
	PL_INT32*		  pp_processBuff_Rx_32b [ APP_AUDIO_RX_CHANNEL_NUMBER_MAX ];		// 32 bit pointer to the Rx output reference buffer

	// USB Tx buffer pointer (data to capture and send to USB Tx circular at end of audio process loop)
	PL_INT8* 		  pp_UsbBuff_Tx_8b 		[ APP_USB_TX_CHANNEL_MAX ];					//  8 bit pointer to the USB TX buffer

#ifdef CONVERSA_PRESENT
	PL_INT8* 		  p_conversaProcessOutBuff_Tx_8b;									//  8 bit pointer to the conversa output buffer Tx
#endif

    // Frame counter
    PL_INT32 frameTime_ms 	 			= 0;			// frame time in ms
    PL_INT32 ledFrameCounter 			= 0;			// frame counter for the Led process
    PL_INT32 ledBlinkHalfPeriode_frame	= 0;			// led blink half period in frame number units

    // tick timer
    TickType_t 	currentTick;

    // Loop
	PL_INT16 iProcess = 0;					    		// loop to run the audio process
    PL_INT16 iLoop	  = 0;

	// temporary
	PL_INT8* 	p_temp1_8b;
	PL_INT16* 	p_temp1_16b;
	PL_INT32* 	p_temp1_32b;
	PL_UINT32 	temp1_u32b;
	PL_FLOAT 	temp1_flt;
	PL_FLOAT 	temp2_flt;

	/******************************************************************
	 *
	 * INIT TASK VARIABLE
	 */
	frameTime_ms =    (p_audioTxPath_handle->configParam.samplePerFrame)			// frame time in ms
					/ (p_audioTxPath_handle->configParam.sampleRate / 1000);

	ledBlinkHalfPeriode_frame = APP_AUDIO_BLINK_LED_PERIOD_MS / frameTime_ms / 2;	// led blink half period in frame number units

	/* Init pointer to process buffer according the audio sample format */
	// Tx process buffer pointer
	for (iLoop=0 ; iLoop < APP_AUDIO_TX_CHANNEL_NUMBER_MAX ; iLoop++)
	{
		pp_processBuff_Tx_8b[iLoop]  = (PL_INT8*)  &g_processBuff_Tx[iLoop][0];  // to address  8 bit format audio sample value
		pp_processBuff_Tx_16b[iLoop] = (PL_INT16*) &g_processBuff_Tx[iLoop][0];  // to address 16 bit format audio sample value
		pp_processBuff_Tx_32b[iLoop] = (PL_INT32*) &g_processBuff_Tx[iLoop][0];  // to address 32 bit format audio sample value
	}

	// Rx reference output buffer pointer
	for (iLoop=0 ; iLoop < APP_AUDIO_RX_CHANNEL_NUMBER_MAX ; iLoop++)
	{
		pp_processBuff_Rx_8b[iLoop]  = (PL_INT8*)  &g_processBuff_Rx[iLoop][0];  // to address  8 bit format audio sample value
		pp_processBuff_Rx_16b[iLoop] = (PL_INT16*) &g_processBuff_Rx[iLoop][0];  // to address 16 bit format audio sample value
		pp_processBuff_Rx_32b[iLoop] = (PL_INT32*) &g_processBuff_Rx[iLoop][0];  // to address 32 bit format audio sample value
	}

	// USB Tx buffer pointer (data to capture and send to USB Tx circular at end of audio process loop)
	for (iLoop=0 ; iLoop < APP_USB_TX_CHANNEL_MAX ; iLoop++)
	{
		pp_UsbBuff_Tx_8b[iLoop] = (PL_INT8*) &g_USBBufferTx[iLoop][0];  		  // to address 8 bit format audio sample value
	}

	/*
	 * START AUDIO CAPTURE
	 *    - start audio capture with the respect of ordering and synchronization
	 */
	retStatusFunc =	INIT_startAudioCapture ( p_audioDefinition_handle );		 // Start the audio capture
	if (retStatusFunc != kStatus_Success)
	{
		PRINTF("[AUDIO_TxRxTask] FAIL audio re organized data from source\r\n");
		retStatus = retStatusFunc;
	}

	// 1st audio task loop on PING event
	p_audioWorkFlow_handle->currentProcessEventGroup_wait = p_audioWorkFlow_handle->processEventPong;

	PRINTF("[AUDIO_TxRxTask] is running...\r\n\n");

	#ifdef CONVERSA_PRESENT
		p_swIpConversa_handle->conversaLicenseTimeOut = PL_FALSE; // by default no time out
	#endif

	while (1)
    {
		/*************************/
    	/*
		 * Wait all received events occurs
		 */
    	/*************************/
		p_audioWorkFlow_handle->currentProcessEventGroup_status = 0;	   			   	   								// reset event received in previous loop

#ifdef MIPS_MEASURE_GPIO
		GPIO_PinWrite(BOARD_USER_MIPS_GPIO_5, BOARD_USER_MIPS_GPIO_5_PIN, 0); // BOARD_USER_MIPS_GPIO_5_PURPOSE_TEXT pin to 0
#endif

		p_audioWorkFlow_handle->currentProcessEventGroup_status =
									xEventGroupWaitBits( 	p_audioWorkFlow_handle->processEventGroup, 					// wait currentProcessEventGroup_status = currentProcessEventGroup_wait
															p_audioWorkFlow_handle->currentProcessEventGroup_wait,      // bits sequence to wait for
															pdFALSE,													// Do not clear bit event group when condition reached, clear occurs in IT
															pdTRUE,														// wait for all bit
															portMAX_DELAY);

#ifdef MIPS_MEASURE_GPIO
		GPIO_PinWrite(BOARD_USER_MIPS_GPIO_5, BOARD_USER_MIPS_GPIO_5_PIN, 1); // BOARD_USER_MIPS_GPIO_5_PURPOSE_TEXT pin to 1
#endif

   		/*************************/
    	/*
		 * Update Ping Pong work flow status
		 */
    	/*************************/

		/* Update currentProcessEventGroup_wait for next event */
    	if (p_audioWorkFlow_handle->currentProcessEventGroup_status == p_audioWorkFlow_handle->processEventPing)    // if Ping events occurs => ping is free / pong is under transfer
		{
    		//PRINTF("[AUDIO_TxRxTask] Ping Event occurs \r\n");
    		p_audioWorkFlow_handle->pingPongTxStatus 			  = AUDIO_WF_PING;  							  // Ping is current buffer to be process
    		p_audioWorkFlow_handle->pingPongRxStatus 			  = AUDIO_WF_PING;  							  // Ping is current buffer to be process
    		p_audioWorkFlow_handle->currentProcessEventGroup_wait = p_audioWorkFlow_handle->processEventPong; 	  // next wait event is Pong
		}
		else if (p_audioWorkFlow_handle->currentProcessEventGroup_status == p_audioWorkFlow_handle->processEventPong)
		{
			//PRINTF("[AUDIO_TxRxTask] Pong Event occurs \r\n");
			p_audioWorkFlow_handle->pingPongTxStatus 			  = AUDIO_WF_PONG;								   // Pong is current buffer to be process
    		p_audioWorkFlow_handle->pingPongRxStatus 			  = AUDIO_WF_PONG;  							   // Pong is current buffer to be process
			p_audioWorkFlow_handle->currentProcessEventGroup_wait = p_audioWorkFlow_handle->processEventPing;      // next wait event is Ping
		}
		else
		{
    		PRINTF("[AUDIO_TxRxTask] FAIL synchronization error");
		}

    	/*************************/
    	/*
    	 * GET Tx data from Tx source
    	 *
    	 * 		Update raw data pointer address
    	 * 		Detect ping pong event
		 * 		Update ping pong mechanism
		 * 		Update pointer address and variables
		 * 		reorganize data in stand alone buffer per channel
		 */
    	/*************************/
    	/* Select ping or pong buffer for the Tx source buffer */

    	if ( p_audioWorkFlow_handle->pingPongTxStatus == AUDIO_WF_PING)
		{
			p_receiveRawBuff_Tx_8b = (PL_INT8*) p_audioTxPath_handle->configParam.bufferHandle.p_pingBufferAddress[0];
		}
		else
		{
			p_receiveRawBuff_Tx_8b = (PL_INT8*) p_audioTxPath_handle->configParam.bufferHandle.p_pongBufferAddress[0];
		}

		/* Reorganize data from interleaved buffer to 1 stand alone buffer per channel */
    	retStatusFunc = toolsReOrganizeAudioData( p_receiveRawBuff_Tx_8b,				// input interleaved data from Tx source
												  pp_processBuff_Tx_8b,					// output standalone buffer well re organise
												  &p_audioTxPath_handle->configParam,   // Tx path configuration
												  AUDIO_DIRECTION_FROM_SOURCE);			// Audio datas are coming from source
    	if (retStatusFunc != kStatus_Success)
    	{
    		PRINTF("[AUDIO_TxRxTask] FAIL audio re organized data from source\r\n");
    		retStatus = retStatusFunc;
    	}

    	/*************************/
    	/*
    	 * GET Rx source data from Rx source
		 *		read Rx data from input source
		 * 		reorganize data in stand alone buffer per channel
		 *
		 /*************************/
    	if ( p_audioRxPath_handle->configParam.source == AUDIO_SRC_TX0 )							   // Rx 0 input is Tx mic 0 input
		{
    		// Get data from Tx input 0
    		memcpy(  (PL_INT8*) pp_processBuff_Rx_8b[0],	(const PL_INT8*) pp_processBuff_Tx_8b[0],	p_audioRxPath_handle->configParam.bytePerFramePerChannel); // destination is Rx input buffer channel 0 / source is Tx input buffer channel 0
		}
    	else if ( p_audioRxPath_handle->configParam.source == AUDIO_SRC_TX0_1 )							// Rx 0/1 input is Tx mic 0/1 input
		{
    		// Get data from Tx input 0 & 1
    		memcpy(  (PL_INT8*) pp_processBuff_Rx_8b[0],	(const PL_INT8*) pp_processBuff_Tx_8b[0],	p_audioRxPath_handle->configParam.bytePerFramePerChannel); // destination is Rx input buffer channel 0 / source is Tx input buffer channel 0
    		memcpy(  (PL_INT8*) pp_processBuff_Rx_8b[1],	(const PL_INT8*) pp_processBuff_Tx_8b[1],   p_audioRxPath_handle->configParam.bytePerFramePerChannel); // destination is Rx input buffer channel 1 / source is Tx input buffer channel 1
		}
    	else if (  ( p_audioRxPath_handle->configParam.source == AUDIO_SRC_USB1 ) 						    // Rx 0/1 input is USB 0/1 Rx circular buffer
    			 ||( p_audioRxPath_handle->configParam.source == AUDIO_SRC_USB2 ))
    	{

    		// Deinterleaved then copy data from USB RX circular buffer to pp_processBuff_Rx_8b
    		retStatusFunc =  toolsCircularBufferRead_I_NI(&g_appHandle.usbTxRx_handle.cirBuffRx,
    													  (PL_UINT8**)pp_processBuff_Rx_8b,
														  p_audioRxPath_handle->configParam.samplePerFrame );
			if (retStatusFunc == kStatus_cbEmpty)
			{
				// PRINTF("[AUDIO_TxRxTask] USB RX path circular buffer empty\r\n");
				retStatus = retStatusFunc;
			}
			else if (retStatusFunc != kStatus_Success)
			{
				PRINTF("[AUDIO_TxRxTask] FAIL to read USB RX path circular buffer\r\n");
				retStatus = retStatusFunc;
			}
    	 }
    	else
    	{
    		PRINTF("[AUDIO_TxRxTask] FAIL to read Rx path source data\r\n");
    		retStatus = kStatus_OutOfRange;
    	}

    	/******************************************************************
    	 * Save data to send to USB Tx
    	 *    - channel process Tx 0 to channel Usb Tx 0
    	 *    - some process are in place processing and erase pp_processBuff_Tx_8b input buffer so save buffer to send before the process
    	 */
    	if (   ( p_audioTxPath_handle->configParam.sink == AUDIO_SINK_USB1 )
    		|| ( p_audioTxPath_handle->configParam.sink == AUDIO_SINK_USB4 )
			|| ( p_audioTxPath_handle->configParam.sink == AUDIO_SINK_USB5 )
			)
    	{
			// save pp_processBuff_Tx_8b into pp_UsbBuff_Tx_8b
    		for (iLoop=0 ; iLoop < APP_MIN(APP_AUDIO_TX_CHANNEL_NUMBER_MAX,APP_USB_TX_CHANNEL_MAX) ; iLoop++)				// run all the Tx buffer
			{
				memcpy( (PL_INT8*) pp_UsbBuff_Tx_8b[iLoop], (const PL_INT8*) pp_processBuff_Tx_8b[iLoop],	p_audioTxPath_handle->configParam.bytePerFramePerChannel);
			}
		}
		else
		{
    		PRINTF("[AUDIO_TxRxTask] FAIL to load USB Tx buffer\r\n");
    		retStatus = kStatus_OutOfRange;
		}

    	/******************************************************************
    	 * Process to call
    	 */
		#ifdef MIPS_MEASURE_GPIO
    		GPIO_PinWrite(BOARD_USER_MIPS_GPIO_6, BOARD_USER_MIPS_GPIO_6_PIN, 1); // BOARD_USER_MIPS_GPIO_6_PURPOSE_TEXT pin to 1
		#endif

    	iProcess = 0;
    	while(iProcess < AUDIO_PROCESS_LIST_MAX)
    	{
    		if (p_audioDefinition_handle->processList[iProcess] == AUDIO_PROC_NO)
    		{
    			/* DO nothing */
    		}
#ifdef CONVERSA_PRESENT
    		/* CONVERSA PROCESS
    		 * 	Tx Channel 0   is used for Tx conversa output
			 * 	Rx Channel 0/1 is used for Rx conversa output
			 */
    		else if (p_audioDefinition_handle->processList[iProcess] == AUDIO_PROC_CONVERSA_TXRX) // if conversa Tx is present in the list
    		{
    			if (   (p_swIpConversa_handle->operatingModeTx == AUDIO_OM_ENABLE)				// if conversa Tx or Rx is enabled in the audio workflow
    				|| (p_swIpConversa_handle->operatingModeRx == AUDIO_OM_ENABLE)
				   )
    			{
    				/* Set conversa output Tx buffer
			    	 *   - if Tx sink is Usb then conversa output is directly USB Tx output channel 0
			    	 *   - else it is process Tx buffer channel 0
			    	 */

					// If USB sink output then copy conversa output to USB Tx buffer 0
					if (   ( p_audioTxPath_handle->configParam.sink == AUDIO_SINK_USB1 )
			    		|| ( p_audioTxPath_handle->configParam.sink == AUDIO_SINK_USB4 )
						|| ( p_audioTxPath_handle->configParam.sink == AUDIO_SINK_USB5 )
						)
			    	{
				    	p_conversaProcessOutBuff_Tx_8b = pp_UsbBuff_Tx_8b[0];			// Conversa Tx output is USB Tx output channel 0

				    	/* USB Tx capture re allocation
				    	 *    - goal is to have Conversa Tx output always on channel 0
				    	 *    - Shift right all USB channel captured => USB buffer (x) = USB buffer (x-1) and USB buffer (0) is now free
				    	 */
						for (iLoop=0 ; iLoop < (APP_USB_TX_CHANNEL_MAX-1) ; iLoop++)
						{
							memcpy( (PL_INT8*) pp_UsbBuff_Tx_8b[ ( APP_USB_TX_CHANNEL_MAX - 1 - iLoop ) ], (const PL_INT8*) pp_UsbBuff_Tx_8b[ ( APP_USB_TX_CHANNEL_MAX - 2 - iLoop ) ],	p_audioTxPath_handle->configParam.bytePerFramePerChannel); // USB buffer (x) = USB buffer (x-1)
						}
			    	}
					else
					{
				    	p_conversaProcessOutBuff_Tx_8b = pp_processBuff_Tx_8b[0];		// Conversa Tx output is process Tx buffer channel 0
					}
					/* Conversa process
					 * 	Tx Channel 0 is used for Tx conversa output
					 * 	Rx Channel 0 is used for Rx conversa output
					 */
					retStatusFunc = swProcessConversa( &p_swIpConversa_handle->conversaPluginParams,		// conversa plugin parameter
    												   pp_processBuff_Tx_8b,								// conversa Tx input pointer of pointer
													   pp_processBuff_Rx_8b,								// conversa Rx input pointer of pointer
													   p_conversaProcessOutBuff_Tx_8b,						// conversa Tx output
													   pp_processBuff_Rx_8b,                     			// conversa Rx output pointer of pointer
													   CONVERSA_PROCESS_FLOAT,								// conversa process in float
													   p_audioTxPath_handle->configParam.samplePerFrame 	// tx/rx frame size in sample
    										   	   	  );
					if (retStatusFunc != kStatus_Success)
    		    	{
						if (retStatusFunc == kStatus_LicenseError)												// if License time out occurs
						{
							// Print only 1 times the license time out message
							if (p_swIpConversa_handle->conversaLicenseTimeOut == PL_FALSE)					// if License time out never occurs yet
							{
								PRINTF("[AUDIO_TxRxTask] FAIL: Conversa license time out occurs\r\n");
							}
							p_swIpConversa_handle->conversaLicenseTimeOut = PL_TRUE;						// Conversa license time out occurs
						}
						else
						{
							PRINTF("[AUDIO_TxRxTask] FAIL: Conversa process\r\n");
							retStatus = retStatusFunc;
						}
    		    	}
    			}
    		}
#endif // end if conversa

    		/* DUMMY PROCESS
    		 */
    		else if (   (p_audioDefinition_handle->processList[iProcess] == AUDIO_PROC_MIPS_5)
    			     || (p_audioDefinition_handle->processList[iProcess] == AUDIO_PROC_MIPS_25)
					 || (p_audioDefinition_handle->processList[iProcess] == AUDIO_PROC_MIPS_50)
					 || (p_audioDefinition_handle->processList[iProcess] == AUDIO_PROC_MIPS_75)
					 || (p_audioDefinition_handle->processList[iProcess] == AUDIO_PROC_MIPS_95)
					 )
    		{
				// time of a frame is 100% Mips load
				if (p_audioDefinition_handle->processList[iProcess] == AUDIO_PROC_MIPS_5) 			// if AUDIO_PROC_MIPS_50 then final load is 5 %
    			{
    				temp1_u32b = frameTime_ms * 5 / 100;											// Final Load = 5% max load
    			}
				else if (p_audioDefinition_handle->processList[iProcess] == AUDIO_PROC_MIPS_25) 	// if AUDIO_PROC_MIPS_50 then final load is 25 %
    			{
    				temp1_u32b = frameTime_ms * 25 / 100;											// Final Load = 25% max load
    			}
				else if (p_audioDefinition_handle->processList[iProcess] == AUDIO_PROC_MIPS_50) 	// if AUDIO_PROC_MIPS_50 then final load is 50 %
    			{
    				temp1_u32b = frameTime_ms * 50 / 100;											// Final Load = 50% max load
    			}
				else if (p_audioDefinition_handle->processList[iProcess] == AUDIO_PROC_MIPS_75) 	// if AUDIO_PROC_MIPS_75 then final load is 75 %
    			{
    				temp1_u32b = frameTime_ms * 75 / 100;											// Final Load = 75% max load
    			}
				else if (p_audioDefinition_handle->processList[iProcess] == AUDIO_PROC_MIPS_95) 	// if AUDIO_PROC_MIPS_95 then final load is 95 %
    			{
    				temp1_u32b = frameTime_ms * 95 / 100;											// Final Load = 95% max load
    			}
    			else
    			{
    				PRINTF("[AUDIO_TxRxTask] Dummy process load not supported\r\n");
    			}

		        // Delay is part of tick so delay will be variable (-/+ 1 tick) according the moment when xTaskGetTickCount() occurs
				temp1_u32b = pdMS_TO_TICKS(temp1_u32b);

		        /* Load with task unblocked */
				//vTaskDelay( temp1_u32b ); // wait x tick

				/* Load with task blocked */
				// dummy function with audio task not suspended
				temp2_flt = 0;
				currentTick = xTaskGetTickCount();
				while( (xTaskGetTickCount() - currentTick)  < temp1_u32b )
				{
					temp2_flt = mipsDummyFunction( xTaskGetTickCount() , temp2_flt);
				}
    		}
    		else
    		{
    			/* Do nothing */
    		}
    		iProcess = iProcess + 1;
    	}

    	#ifdef MIPS_MEASURE_GPIO
    		GPIO_PinWrite(BOARD_USER_MIPS_GPIO_6, BOARD_USER_MIPS_GPIO_6_PIN, 0); // BOARD_USER_MIPS_GPIO_6_PURPOSE_TEXT pin to 0
		#endif

		/*******************************************************************
		 *
		 * Check License error during processing
		 *     - if occurs then set Rx and Tx buffer to 0
		 */
		#ifdef CONVERSA_PRESENT
    		// Check if Conversa License Time Out occurs
			if (p_swIpConversa_handle->conversaLicenseTimeOut == PL_TRUE)							// if Conversa License time out occurs
			{
				// Set buffer Tx value to 0
				for (iLoop=0 ; iLoop < APP_AUDIO_TX_CHANNEL_NUMBER_MAX ; iLoop++)					// run all the USB Tx buffer
				{
					memset((PL_INT8*) pp_processBuff_Tx_8b[iLoop],0,p_audioTxPath_handle->configParam.bytePerFramePerChannel);
				}
				// Set buffer Tx value to 0
				for (iLoop=0 ; iLoop < APP_AUDIO_RX_CHANNEL_NUMBER_MAX ; iLoop++)					// run all the USB Rx buffer
				{
					memset((PL_INT8*) pp_processBuff_Rx_8b[iLoop],0,p_audioRxPath_handle->configParam.bytePerFramePerChannel);
				}
			}
		#endif

		/*******************************************************************
    	 *
    	 * SEND TX DATA TO TX SINK
    	 */
    	if (   ( p_audioTxPath_handle->configParam.sink == AUDIO_SINK_USB1 )
    		|| ( p_audioTxPath_handle->configParam.sink == AUDIO_SINK_USB4 )
			|| ( p_audioTxPath_handle->configParam.sink == AUDIO_SINK_USB5 )
			)
    	{
    		/**************************/
    		/*
    		 *  Interleave N channel audio data to 1 interleave audio buffer
    		 */
    		// the optional flag will be use to propagate the startRecording state inside CirBuff structure
    		g_appHandle.usbTxRx_handle.cirBuffTx.flagOptional = g_appHandle.usbTxRx_handle.audioUnified.startRecording;
			retStatusFunc = toolsCircularBufferWrite_NI_I(&g_appHandle.usbTxRx_handle.cirBuffTx,
													     (PL_UINT8**)pp_UsbBuff_Tx_8b,							// send data pre saved in the audio process loop
													      p_audioTxPath_handle->configParam.samplePerFrame);

			if (retStatusFunc == kStatus_cbFull)
			{
				//PRINTF("[AUDIO_TxRxTask] USB TX path circular buffer full\r\n");
				retStatus = retStatusFunc;
				g_appHandle.usbTxRx_handle.audioUnified.startRecording = 0; // Flag indicate if device is recording or not

			}
			else if (retStatusFunc != kStatus_Success)
			{
				PRINTF("[AUDIO_TxRxTask] FAIL to write USB TX path circular buffer\r\n");
				retStatus = retStatusFunc;
			}
    	}
    	else
    	{
    		// DO nothing
    	}

    	// Read position adjustment needed in case recording still not started
    	// We adjust the read pointer when recording is not started to always let a half buffer between read and write pointer to avoid underflow overrun when recording start
    	if (   (g_appHandle.usbTxRx_handle.cirBuffTx.flagMidFull       ==1)
    		&& (g_appHandle.usbTxRx_handle.audioUnified.startRecording ==0))
		{
			toolsCircularBufferAdjustRdPos(&g_appHandle.usbTxRx_handle.cirBuffTx,
										   p_audioTxPath_handle->configParam.bytePerFrame );
		}

    	/*************************/
    	/*
    	 * SEND RX DATA TO RX SINK
    	 *
		 *   - Select ping or pong buffer
		 *   - Copy data to interleave buffer
		 *   - Add data to Edma queue
		 */
    	/*************************/

    	/* Corner case:
    	 * if Rx channel input is 1 channel but output is 2 channels then create 2 channels by copy
    	 * This permits to play on 2 speakers to increase the loudness
    	 */
		if (  	( p_audioRxPath_handle->configParam.sink 	== AUDIO_SINK_SPEAKER2  )  // Rx path output channel is 2
			 && ( p_audioRxPath_handle->configParam.source 	== AUDIO_SRC_USB1      )   // Rx path input channel is 1
			 && ( APP_AUDIO_RX_CHANNEL_NUMBER_MAX 	     	>= 2 				   	)  // Rx Buffer reserved space is >= 2 channels
			)
    	{
    		memcpy(  (PL_INT8*) pp_processBuff_Rx_8b[1], (const PL_INT8*) pp_processBuff_Rx_8b[0],	p_audioRxPath_handle->configParam.bytePerFramePerChannel); // copy channel 0 to channel 1 to have signal on the 2nd speaker
    	}

    	/* Select ping or pong buffer for the Rx sink buffer */
    	if ( p_audioWorkFlow_handle->pingPongRxStatus == AUDIO_WF_PING)
		{
			p_temp1_8b  = (PL_INT8*)  p_audioRxPath_handle->configParam.bufferHandle.p_pingBufferAddress[0];
			p_temp1_16b = (PL_INT16*) p_audioRxPath_handle->configParam.bufferHandle.p_pingBufferAddress[0];
			p_temp1_32b = (PL_INT32*) p_audioRxPath_handle->configParam.bufferHandle.p_pingBufferAddress[0];
		}
		else
		{
			p_temp1_8b  = (PL_INT8*)  p_audioRxPath_handle->configParam.bufferHandle.p_pongBufferAddress[0];
			p_temp1_16b = (PL_INT16*) p_audioRxPath_handle->configParam.bufferHandle.p_pongBufferAddress[0];
			p_temp1_32b = (PL_INT32*) p_audioRxPath_handle->configParam.bufferHandle.p_pongBufferAddress[0];
		}

		/* Reorganize data to send to the sink */
		retStatusFunc = toolsReOrganizeAudioData( p_temp1_8b,			 				// outut interleaved data to send to sink
												  pp_processBuff_Rx_8b,					// input standalone buffer
												  &p_audioRxPath_handle->configParam,   // Rx path configuration
												  AUDIO_DIRECTION_TO_SINK);			    // Audio datas are going to sink

		if (retStatusFunc != kStatus_Success)
    	{
    		PRINTF("[AUDIO_TxRxTask] FAIL audio re organized data to sink\r\n");
    		retStatus = retStatusFunc;
    	}

		/*
		 * Led blink every 5 sec
		 */
		ledFrameCounter = ledFrameCounter + 1;
		if (ledFrameCounter > ledBlinkHalfPeriode_frame) { ledFrameCounter = 0; USER_LED_TOGGLE(); }

    } // end while(1)
}


/*
 * mips Dummy Function
 *    - do dummy float computation
 *    - simulate FPU usage
 *    - simulate stack consumption
 */
PL_FLOAT mipsDummyFunction(TickType_t tickNumber , PL_FLOAT value_flt)
{
	PL_FLOAT tab_flt[MIPS_DUMMY_FUNCTION_STACK_USAGE];
	PL_INT32 iLoop;

	tab_flt[0] = 0.05;
	for (iLoop = 1; iLoop< MIPS_DUMMY_FUNCTION_STACK_USAGE; iLoop++)
	{
		tab_flt[iLoop] = tab_flt[iLoop-1] * (PL_FLOAT)(tickNumber) + value_flt;
		if (tab_flt[iLoop] > 10000) {tab_flt[iLoop] = 0.05;}
	}
	return tab_flt[MIPS_DUMMY_FUNCTION_STACK_USAGE-1];
}

