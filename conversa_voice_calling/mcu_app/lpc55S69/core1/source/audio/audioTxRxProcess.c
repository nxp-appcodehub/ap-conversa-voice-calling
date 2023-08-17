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
#include "fsl_mailbox.h"
#include "mcmgr.h"

//Application headers
#include "appGlobal.h"
#include "usb_init.h"
#include "audioTxRxProcess.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
// This structure is shared by Core0 & Core1. It is stored in a memory area know by Core1
extern volatile T_CommonVarSharedByCore0AndCore1 CommonVarSharedByCore0AndCore1;
// this pointers handle I2s buffers to receive/send data
extern PL_UINT32 *p_I2SBuffRX;
extern PL_UINT32 *p_I2SBuffTX;

/*****************************************************************************************
 * Function
 *****************************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/


/*******************************************************************************
 * Code
 ******************************************************************************/
__attribute__((__section__("RamFunction")))
status_t audioTxRxProcess(AUDIO_definition_st* p_audioDefinition_handle)
{
	// error handle
	status_t 	 			 retStatus 	 	 = kStatus_Success;
	status_t 	 			 retStatusFunc   = kStatus_Success;
#if RX_PATH_PRESENT
	PL_INT8* 		  pp_processBuff_Rx_8b  [APP_AUDIO_RX_CHANNEL_NUMBER_MAX];		//  8 bit pointer to the Rx output reference buffer
#endif
	PL_INT32* 	      pp_processBuff_Tx_32b [APP_USB_TX_CHANNEL_MAX];
	PL_INT8* 	      pp_processBuff_Tx_8b  [APP_USB_TX_CHANNEL_MAX];
	// Temp Pointers
	PL_INT8* 	      pp_temp_Tx_8b         [APP_USB_TX_CHANNEL_MAX];
#if RX_PATH_PRESENT
	PL_INT8* 	      pp_temp_RxIn_8b       [APP_AUDIO_RX_CHANNEL_NUMBER_MAX];
	PL_INT8* 	      pp_temp_RxOut_8b      [APP_AUDIO_RX_CHANNEL_NUMBER_MAX];
#endif
	// Loop
	PL_INT16 iProcess = 0;					    									// loop to run the audio process
	PL_INT16 iLoop	  = 0;
	//check for null pointer
	if (p_audioDefinition_handle == NULL)
	{
		return kStatus_NullPointer;
	}
	/***************************************************************************************/
	/* PING/PONG update
	/***************************************************************************************/
    // Update Process PING/PONG wait  + assign buffers + reset process status

    if (p_audioDefinition_handle->audioWorkFlow_handle.currentProcessEventGroup_wait == p_audioDefinition_handle->audioWorkFlow_handle.processEventPing) // we were waiting PING,
    {
#if RX_PATH_PRESENT
    	p_I2SBuffRX												 						  = g_sinkBuffPingRx;
#endif
    	p_I2SBuffTX												 						  = g_sourceRawBuffPingTx;
    	p_audioDefinition_handle->audioWorkFlow_handle.currentProcessEventGroup_wait 	  = p_audioDefinition_handle->audioWorkFlow_handle.processEventPong;  // we are waiting PONG ( TCD is writing in PONG)
#ifdef MIPS_MEASURE_GPIO
    	GPIO_4_Up();
#endif
    }
    else																																				// we
    {
#if RX_PATH_PRESENT
    	p_I2SBuffRX																		   = g_sinkBuffPongRx;
#endif
    	p_I2SBuffTX												 						   = g_sourceRawBuffPongTx;
    	p_audioDefinition_handle->audioWorkFlow_handle.currentProcessEventGroup_wait       = p_audioDefinition_handle->audioWorkFlow_handle.processEventPing;
#ifdef MIPS_MEASURE_GPIO
    	GPIO_4_Dn();
#endif
    }

    /* Init pointer to process buffer according the audio sample format */
    // Tx reference output buffer pointer
    for (iLoop=0 ; iLoop < APP_USB_TX_CHANNEL_MAX ; iLoop++)
	{
		pp_processBuff_Tx_32b[iLoop] = (PL_INT32*) &g_processBuff_Tx[iLoop][0];        // to address 32 bit format audio sample value
		pp_processBuff_Tx_8b [iLoop] = (PL_INT8*)  &g_processBuff_Tx[iLoop][0];        // to address 32 bit format audio sample value
		pp_temp_Tx_8b        [iLoop] = (PL_INT8*)  &CommonVarSharedByCore0AndCore1.pp_ConversaProcessBuff_TxIn[CommonVarSharedByCore0AndCore1.buffToChoose][iLoop][0];
	}
#if RX_PATH_PRESENT
	// Rx reference output buffer pointer
	for (iLoop=0 ; iLoop < APP_AUDIO_RX_CHANNEL_NUMBER_MAX ; iLoop++)
	{
		pp_processBuff_Rx_8b [iLoop]  = (PL_INT8*)  &g_processBuff_Rx[iLoop][0];  		// to address  8 bit format audio sample value
		pp_temp_RxOut_8b     [iLoop]  = (PL_INT8*)  &CommonVarSharedByCore0AndCore1.pp_ConversaProcessBuff_RxOut[CommonVarSharedByCore0AndCore1.buffToChoose][iLoop][0];
	}

	pp_temp_RxIn_8b      [0]  = (PL_INT8*)  &CommonVarSharedByCore0AndCore1.pp_ConversaProcessBuff_RxIn [CommonVarSharedByCore0AndCore1.buffToChoose][0][0];
#endif
	/*************************/
	/*
	 * GET Tx data from Tx source
	 *		mask mic buffers to filter unneeded content
	 * 		reorganize data in stand alone buffer per channel
	 * 		Copy mic buffers into circular buffers before SW process
	 */
	/*************************/
	/*
	 * WORKAROUND: only for RetuneDSP Mic array boards at 16k
	 * 			   for 16k of LRCLK, the BCLK is 1.023 Mhz. it is not enough for these mics that require a PDM_CLK > 1 Mhz. Sometimes the signal can be lost.
	 * 			   Workaround : Mics are acquired at 32k, then Fixed point SSRC to 16k at this point
	 */
	if (   ( g_appHandle.platformAndMaterials == APP_PLATFORM_MATERIALS_LPC55S69_TB2136_RDSPMICARRAY)
		&& ( g_appHandle.audioDefinition.audioTxPath_handle.configParam.sampleRate == 16000)	)
	{
		 for (iProcess=0 ; iProcess < g_appHandle.audioDefinition.audioTxPath_handle.configParam.samplePerFrame* 2 *g_appHandle.audioDefinition.audioTxPath_handle.configParam.channelNumber  ; iProcess++)
		 {
			 /* The RetuneDSP Mic array is using a pdmToI2S converter. This converter has a dynamic of 20 bits/samples.
			  *  We need to filter the 12 LSB to ensure we don't get wrong content 										*/
			 g_TXRawBuff[iProcess] = p_I2SBuffTX[iProcess] & RDSP_MICARRAY_MASK ;
		 }
		 /* SSRC Process */
		 SSRC_ReturnStatus_en ReturnStatus;
		 /* In this case Mics are at 32k, so do SSRC to 16k */
		 ReturnStatus = SSRC_Process_D32(  &SSRC_Instance,
				 	 	 		  	  	  (LVM_INT32*)&g_TXRawBuff[0],
										  (LVM_INT32*)p_I2SBuffTX);

		 if (ReturnStatus != SSRC_OK)
		 {
			 PRINTF("Error code %d returned by the SSRC_Process function\r\n",(LVM_INT16)ReturnStatus);
		  }
	}
	else if ( g_appHandle.platformAndMaterials == APP_PLATFORM_MATERIALS_LPC55S69_TB2136_RDSPMICARRAY)
	{
		for (iProcess=0 ; iProcess < g_appHandle.audioDefinition.audioTxPath_handle.configParam.samplePerFrame*g_appHandle.audioDefinition.audioTxPath_handle.configParam.channelNumber  ; iProcess++)
		 {
			/* The RetuneDSP Mic array is using a pdmToI2S converter. This converter has a dynamic of 20 bits/samples.
			 *  We need to filter the 12 LSB to ensure we don't get wrong content 										*/
			p_I2SBuffTX[iProcess] = p_I2SBuffTX[iProcess] & RDSP_MICARRAY_MASK ;
		 }
	}
	else
	{
		/* other materials */
	}
	if(g_appHandle.audioDefinition.ConfigWithConversa == PL_TRUE)
	{
		/* Save previous frame to send it to TX Out */
		// we need to shift TX_mic buffers to let first place for CONVERSA_TX_OUT. Then we could send by USB [CONVERSA_TX_OUT, MIC1_RAW,...MICX_RAW]
		if (   ( g_appHandle.audioDefinition.audioTxPath_handle.configParam.sink == AUDIO_SINK_USB1 )
			|| ( g_appHandle.audioDefinition.audioTxPath_handle.configParam.sink == AUDIO_SINK_USB4 )
			)
		{
			for (iLoop=0 ; iLoop < (APP_USB_TX_CHANNEL_MAX-1) ; iLoop++)
			{
				memcpy( (PL_INT8*)       pp_processBuff_Tx_8b[ ( APP_USB_TX_CHANNEL_MAX - 1 - iLoop ) ],
						(const PL_INT8*) CommonVarSharedByCore0AndCore1.pp_ConversaProcessBuff_TxIn[CommonVarSharedByCore0AndCore1.buffToChoose][ ( APP_USB_TX_CHANNEL_MAX - 2 - iLoop ) ],
						p_audioDefinition_handle->audioTxPath_handle.configParam.samplePerFrame * p_audioDefinition_handle->audioTxPath_handle.configParam.bitPerSample/8);
			}
		}
		/* De-interleave Mics buffer */
		retStatusFunc = toolsCircularBufferDeInterleaveData_32b(p_I2SBuffTX,
																(PL_UINT32 **) pp_temp_Tx_8b,
																p_audioDefinition_handle->audioTxPath_handle.configParam.samplePerFrame,
																p_audioDefinition_handle->audioTxPath_handle.configParam.channelNumber,
																0);
	}
	else
	{
		/* SSRC */
		/* De-interleave Mics buffer */
		retStatusFunc = toolsCircularBufferDeInterleaveData_32b(p_I2SBuffTX,
																(PL_UINT32 **)pp_processBuff_Tx_32b,
																p_audioDefinition_handle->audioTxPath_handle.configParam.samplePerFrame,
																p_audioDefinition_handle->audioTxPath_handle.configParam.channelNumber,
																0);
	}
	if (retStatusFunc != kStatus_Success)
	{
		PRINTF("[AUDIO_TxRxProcess] FAIL audio de-interleave TX data from source\r\n");
		retStatus = retStatusFunc;
	}
	/*******************************************************************
	 *
	 * SEND TX DATA TO TX SINK
	 *
	 *******************************************************************/
	if (   ( g_appHandle.audioDefinition.audioTxPath_handle.configParam.sink == AUDIO_SINK_USB1 )
		|| ( g_appHandle.audioDefinition.audioTxPath_handle.configParam.sink == AUDIO_SINK_USB4 )
		)
	{
		if(g_appHandle.audioDefinition.ConfigWithConversa == PL_TRUE)
		{
			//Read CONVERSA_TX_OUT[0] Circular buffer. Conversa is always 1 TX OUT channel
			memcpy( (PL_INT8*)       pp_processBuff_Tx_8b[0],
					(const PL_INT8*) CommonVarSharedByCore0AndCore1.pp_ConversaProcessBuff_TxOut[CommonVarSharedByCore0AndCore1.buffToChoose][0],
					p_audioDefinition_handle->audioTxPath_handle.configParam.samplePerFrame * p_audioDefinition_handle->audioTxPath_handle.configParam.bitPerSample/8);
		}
		/**************************/
		/*
		 *  Interleave N channel audio data to 1 interleave audio buffer
		 */
		// the optional flag will be use to propagate the startRecording state inside CirBuff structure
		g_appHandle.UsbCompositeDev_handle.cirBuffTx.flagOptional = g_appHandle.UsbCompositeDev_handle.audioUnified.startRec;
		retStatusFunc = toolsCircularBufferWrite_NI_I(&g_appHandle.UsbCompositeDev_handle.cirBuffTx,
													 (PL_UINT8**)pp_processBuff_Tx_8b,							// send data pre saved in the audio process loop
													 p_audioDefinition_handle->audioTxPath_handle.configParam.samplePerFrame);
		if (retStatusFunc == kStatus_cbFull)
		{
#if _DEBUG
			PRINTF("[AUDIO_TxRxProcess] USB TX path circular buffer full\r\n");
#endif
			retStatus = retStatusFunc;
			g_appHandle.UsbCompositeDev_handle.audioUnified.startRec = 0; // Flag indicate if device is recording or not

		}
		else if (retStatusFunc != kStatus_Success)
		{
			PRINTF("[AUDIO_TxRxProcess] FAIL to write USB TX path circular buffer\r\n");
			retStatus = retStatusFunc;
		}
	}
	else
	{
		// DO nothing
	}
	// Read position adjustment needed in case recording still not started
	// We adjust the read pointer when recording is not started to always let a half buffer between read and write pointer to avoid underflow overrun when recording start
	if (   (g_appHandle.UsbCompositeDev_handle.cirBuffTx.flagMidFull       ==1)
		&& (g_appHandle.UsbCompositeDev_handle.audioUnified.startRec ==0))
	{
		toolsCircularBufferAdjustRdPos(&g_appHandle.UsbCompositeDev_handle.cirBuffTx,
				p_audioDefinition_handle->audioTxPath_handle.configParam.samplePerFrame* (p_audioDefinition_handle->audioTxPath_handle.configParam.bitPerSample/8) * g_appHandle.UsbCompositeDev_handle.cirBuffTx.cbChannelNum );
	}
#if RX_PATH_PRESENT
	/*************************/
	/*
	 * GET Rx source data from Rx source
	 *		read Rx data from input source
	 * 		reorganize data in stand alone buffer per channel
	 *
	 /*************************/
	if ( p_audioDefinition_handle->audioRxPath_handle.configParam.source == AUDIO_SRC_TX0 )							       // Rx 0 input is Tx mic 0 input
	{
		// Get data from Tx input 0
		memcpy(  (PL_INT8*) pp_processBuff_Rx_8b[0],
				 (const PL_INT8*) pp_processBuff_Tx_32b[0],
				 p_audioDefinition_handle->audioRxPath_handle.configParam.samplePerFrame* (p_audioDefinition_handle->audioRxPath_handle.configParam.bitPerSample/8)); // destination is Rx input buffer channel 0 / source is Tx input buffer channel 0
	}
	else if ( p_audioDefinition_handle->audioRxPath_handle.configParam.source == AUDIO_SRC_TX0_1 )							// Rx 0/1 input is Tx mic 0/1 input
	{
		// Get data from Tx input 0 & 1
		memcpy( (PL_INT8*) pp_processBuff_Rx_8b[0],
				(const PL_INT8*) pp_processBuff_Tx_32b[0],
				p_audioDefinition_handle->audioRxPath_handle.configParam.samplePerFrame* (p_audioDefinition_handle->audioRxPath_handle.configParam.bitPerSample/8)); // destination is Rx input buffer channel 0 / source is Tx input buffer channel 0
		memcpy(  (PL_INT8*) pp_processBuff_Rx_8b[1],
				 (const PL_INT8*) pp_processBuff_Tx_32b[1],
				 p_audioDefinition_handle->audioRxPath_handle.configParam.samplePerFrame* (p_audioDefinition_handle->audioRxPath_handle.configParam.bitPerSample/8)); // destination is Rx input buffer channel 0 / source is Tx input buffer channel 0
	}
	else if (  (   p_audioDefinition_handle->audioRxPath_handle.configParam.source == AUDIO_SRC_USB1 ) 	// Rx 0/1 input is USB 0/1 Rx circular buffer
			 ||(   p_audioDefinition_handle->audioRxPath_handle.configParam.source == AUDIO_SRC_USB2 ))
	{
		if(g_appHandle.audioDefinition.ConfigWithConversa == PL_TRUE)
		{
			// Deinterleaved then copy data from USB RX circular buffer to pp_processBuff_Rx_8b
			retStatusFunc =  toolsCircularBufferRead_I_NI(&g_appHandle.UsbCompositeDev_handle.cirBuffRx,
														  (PL_UINT8**)pp_temp_RxIn_8b,
														  p_audioDefinition_handle->audioRxPath_handle.configParam.samplePerFrame );
		}
		else
		{
			// Deinterleaved then copy data from USB RX circular buffer to pp_processBuff_Rx_8b
			retStatusFunc =  toolsCircularBufferRead_I_NI(&g_appHandle.UsbCompositeDev_handle.cirBuffRx,
														  (PL_UINT8**)pp_processBuff_Rx_8b,
														  p_audioDefinition_handle->audioRxPath_handle.configParam.samplePerFrame );
		}

		if (retStatusFunc == kStatus_cbEmpty)
		{
#if _DEBUG
			PRINTF("[AUDIO_TxRxProcess] USB RX path circular buffer empty\r\n");
#endif
			retStatus = retStatusFunc;
		}
		else if (retStatusFunc != kStatus_Success)
		{
			PRINTF("[AUDIO_TxRxProcess] FAIL to read USB RX path circular buffer\r\n");
			retStatus = retStatusFunc;
		}
	}
	else
	{
		PRINTF("[AUDIO_TxRxProcess] FAIL to read Rx path source data\r\n");
		retStatus = kStatus_OutOfRange;
	}
	/*************************/
	/*
	 * SEND RX DATA TO SINK
	 *
	 */
	/*************************/
	if ((p_audioDefinition_handle->audioRxPath_handle.configParam.sink == AUDIO_SINK_SPEAKER1 )
	 ||(p_audioDefinition_handle->audioRxPath_handle.configParam.sink == AUDIO_SINK_SPEAKER2 ))
	{
		if(g_appHandle.audioDefinition.ConfigWithConversa == PL_TRUE)
		{

			retStatusFunc = toolsCircularBufferInterleaveData_32b((PL_UINT32 **)pp_temp_RxOut_8b,
																  p_I2SBuffRX,
																  p_audioDefinition_handle->audioRxPath_handle.configParam.samplePerFrame,
																  APP_AUDIO_RX_CHANNEL_NUMBER_MAX,
																  0);
		}
		else
		{
			retStatusFunc = toolsCircularBufferInterleaveData_32b((PL_UINT32 **)pp_processBuff_Rx_8b,
																  p_I2SBuffRX,
																  p_audioDefinition_handle->audioRxPath_handle.configParam.samplePerFrame,
																  APP_AUDIO_RX_CHANNEL_NUMBER_MAX,
																  0);
		}
		if (retStatusFunc != kStatus_Success)
		{

			PRINTF("[AUDIO_TxRxProcess] FAIL audio interleave RX data to sink\r\n");
			retStatus = retStatusFunc;
		}
	}
#endif
	/*
	 * Send request for Conversa processing: at this point we get a new frame so Core1 will ask Core0 to process this new frame
	 *
	 */
	if(g_appHandle.audioDefinition.ConfigWithConversa == PL_TRUE)
	{
		if (CommonVarSharedByCore0AndCore1.audioDefinition.swIpConversa_handle.Core0Busy == PL_FALSE)
		{
			// buffToChoose is a buffer selector. If Core1 feed/get [0], so Core0 will process [1]
			CommonVarSharedByCore0AndCore1.buffToChoose = 1-CommonVarSharedByCore0AndCore1.buffToChoose;
			CommonVarSharedByCore0AndCore1.audioDefinition.swIpConversa_handle.Core0Busy = PL_TRUE;
			MCMGR_TriggerEvent(kMCMGR_Core0_AskConversaProcess, (uint16_t) 0);
		}
		else
		{
			PRINTF("[AUDIO_TxRxProcess] Core0 is taking too much time to Process\r\n");
			return kStatus_Timeout;
		}
	}
	return retStatus;
}

