/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// general
#include "appGlobal.h"

// memory section replacement
#include <cr_section_macros.h>

// audio
#include "audio.h"

// CMSIS lib
#include "arm_math.h"

// GPIO for debug and MIPS measure on J26
#ifdef MIPS_MEASURE_GPIO
	#include "fsl_gpio.h"
#endif

#ifdef CONVERSA_PRESENT
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            swProcessConversa			   										 	 */
/*                                                                                               */
/* DESCRIPTION: process input audio data by conversa 			  	 					 		 */
/*                                                                                               */
/* PARAMETERS:                                                                                   */
/*         conversaProcess conversa process to use (float or Q31)                              	 */
/*************************************************************************************************/
__RAMFUNC(RAM) // force to put the code instruction of this function in RAM (FLEXRAMRAM_ITC)
status_t swProcessConversa( rdsp_conversa_plugin_t*   p_conversaPluginParams,
							PL_INT8**   			  pp_inputAudioData_Tx,
						    PL_INT8**   			  pp_inputAudioData_Rx,
							PL_INT8*   			  	  p_outputAudioData_Tx,
							PL_INT8**   		  	  pp_outputAudioData_Rx,
							CONVERSA_processFormat_en conversaProcess,
							PL_INT32				  inputAudioDataSize_sample
						   )
{
	status_t   retStatus 		 = kStatus_Success;
	status_t   retStatusFunc	 = kStatus_Success;
	RdspStatus retStatusRdsp 	 = OK;

	// input float buffer
	PL_FLOAT*  pp_inputAudioData_Tx_FLT [APP_AUDIO_TX_CHANNEL_NUMBER_MAX];
	PL_FLOAT*  pp_inputAudioData_Rx_FLT [APP_AUDIO_RX_CHANNEL_NUMBER_MAX];

	// output pointer
	PL_FLOAT*  p_outputAudioData_Tx_FLT = NULL; 	// conversa out pointer
	PL_FLOAT** pp_outputAudioData_Rx_FLT= NULL ; 	// conversa out pointer

	PL_FLOAT** p_currentSensingInput = NULL;		// current sensing input

	PL_UINT16 iChannel;

	PL_UINT32 iDebug1;
	PL_UINT32 iDebug2;
	volatile q31_t* pDebug_Q31;

	/***************************/
	/* CHECK PARAMETER POINTER */
	// Check input pointer
	if ( (pp_inputAudioData_Tx == NULL) || (pp_inputAudioData_Rx == NULL) || (p_outputAudioData_Tx == NULL) || (pp_outputAudioData_Rx == NULL))
	{
		return kStatus_NullPointer;
	}
	// Check all pp_inputAudioData pointer
	for (iChannel = 0; iChannel < p_conversaPluginParams->num_mics; iChannel++)
	{
		if (pp_inputAudioData_Tx[iChannel] == NULL) {return kStatus_NullPointer;}
	}
	// Check all pp_inputAudioReferenceData pointer
	for (iChannel = 0; iChannel < p_conversaPluginParams->num_rx_in; iChannel++)
	{
		if (pp_inputAudioData_Rx[iChannel]  == NULL) {return kStatus_NullPointer;}
	}
	// Check all pp_inputAudioReferenceData pointer
	for (iChannel = 0; iChannel < p_conversaPluginParams->num_spks; iChannel++)
	{
		if (pp_outputAudioData_Rx[iChannel] == NULL) {return kStatus_NullPointer;}
	}

	/********************/
	/* CHECK PARAMETERS */
	// Is current sensing is use for aec ?
	if (p_conversaPluginParams->config.aec_uses_current_sensing == CONVERSA_DISABLE)
	{
		p_currentSensingInput = NULL;
	}
	else
	{
		PRINTF("FAIL: aec_uses_current_sensing in swProcessConversa function out of range");
		retStatus = kStatus_OutOfRange;
	}


	/************************************/
	/* CONVERSA PROCESS                 */
	/*    - Float or Q31 format process */

	/* Conversa FLOAT process
	 *    - Float conversion
	 *    - Float process
	 *    - Read Tx data
	 */

	if (conversaProcess == CONVERSA_PROCESS_FLOAT)
	{
		for (iChannel=0 ; iChannel < APP_AUDIO_TX_CHANNEL_NUMBER_MAX ; iChannel++)
		{
			 pp_inputAudioData_Tx_FLT[iChannel] = (PL_FLOAT*) &g_processBuff_Tx[iChannel][0];
		}

		// Rx reference output buffer pointer
		for (iChannel=0 ; iChannel < APP_AUDIO_RX_CHANNEL_NUMBER_MAX ; iChannel++)
		{
			 pp_inputAudioData_Rx_FLT[iChannel] = (PL_FLOAT*) &g_processBuff_Rx[iChannel][0];
		}

		/* Tx convert input channel to data Q31 to float */
		for (iChannel=0; iChannel<p_conversaPluginParams->num_mics; iChannel++)
		{

			arm_q31_to_float (	(const q31_t*) 	  pp_inputAudioData_Tx[iChannel],
												  pp_inputAudioData_Tx_FLT[iChannel],
												  inputAudioDataSize_sample
							 );
		}

		/* Rx convert reference data Q31 to float */
		for (iChannel=0; iChannel<p_conversaPluginParams->num_rx_in; iChannel++)
		{
			arm_q31_to_float (	(const q31_t*) 	pp_inputAudioData_Rx[iChannel],
												pp_inputAudioData_Rx_FLT[iChannel],
												inputAudioDataSize_sample
							 );
		}

		/* Conversa Tx Rx process */
		retStatusRdsp = RdspConversa_Plugin_Process( 	p_conversaPluginParams,
													    pp_inputAudioData_Tx_FLT,
														pp_inputAudioData_Rx_FLT,
														p_currentSensingInput
													);

		if (retStatusRdsp != OK) 						// if return status not OK
		{
			if (retStatusRdsp == LICENSE_EXPIRED)       // if license expired occurs
			{
				retStatus = kStatus_LicenseError;
			}
			else
			{
				PRINTF("FAIL: conversa process error %d\r\n",retStatusRdsp);
				retStatus = kStatus_Fail;
			}
		}

		/* Handle output data
		 *    - Get conversa Tx out data address
		 *    - convert float to fix
		 */
		// get Tx data address
		p_outputAudioData_Tx_FLT = RdspConversa_Plugin_GetTxOut(p_conversaPluginParams);

		if (p_outputAudioData_Tx_FLT != PL_NULL)
		{
			// convert output ConversaTx data float to Q31
			arm_float_to_q31 (	(const PL_FLOAT*) 	p_outputAudioData_Tx_FLT,
								(q31_t*) 			p_outputAudioData_Tx,
								inputAudioDataSize_sample
						 	 );
		}
		else
		{
			PRINTF("FAIL: conversaProcess return NULL pointer\r\n");
			retStatus = kStatus_OutOfRange;
		}

		/* Handle output data
		 *    - Get conversa Tx out data address
		 *    - convert float to fix
		 */
		// get Rx data address
		pp_outputAudioData_Rx_FLT = RdspConversa_Plugin_GetRxOut(p_conversaPluginParams);

		if (pp_outputAudioData_Rx_FLT[0] != PL_NULL)
		{
		 // convert output ConversaRx data float to Q31
		 arm_float_to_q31 (	(const PL_FLOAT*) 	pp_outputAudioData_Rx_FLT[0],   	// channel 0 Rx
							(q31_t*) 			pp_outputAudioData_Rx[0],
							inputAudioDataSize_sample
						  );
		}
		else
		{
			PRINTF("FAIL: conversaProcess return NULL pointer\r\n");
			retStatus = kStatus_OutOfRange;
		}

		if ( p_conversaPluginParams->num_spks == 2 )
		{
			if (pp_outputAudioData_Rx_FLT[1] != PL_NULL)
			{
				// convert output ConversaRx data float to Q31
				arm_float_to_q31 (	(const PL_FLOAT*) 	pp_outputAudioData_Rx_FLT[1],   // channel 0 Rx
									(q31_t*) 			pp_outputAudioData_Rx[1],
														inputAudioDataSize_sample
								 );
			}
			else
			{
				PRINTF("FAIL: conversaProcess return NULL pointer\r\n");
				retStatus = kStatus_OutOfRange;
			}
		 }
	} // end if (conversaProcess == CONVERSA_PROCESS_FLOAT)
	else
	{
		PRINTF("FAIL: conversaProcess in swProcessConversa function out of range\r\n");
		retStatus = kStatus_OutOfRange;
	}
	return retStatus;
}

#endif
