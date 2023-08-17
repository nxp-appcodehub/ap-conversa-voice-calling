/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include <swProcess/swProcessConversa.h>
#include "fsl_debug_console.h"
#include "fsl_gpio.h"
#include "pin_mux.h"
#include "board.h"
#include "mcmgr.h"
#include "fsl_power.h"
#include "fsl_mailbox.h"
#include "fsl_common_arm.h"

#include "appGlobal.h"


#include "RdspConversaPlugin.h"
#include "RdspConversaPluginConfig.h"
#include "Core0MCMGR.h"
#include "audio.h"

#if MEASURE_STACK_USAGE
#include "RdspStackCheck.h"
#endif

#if CONVERSAML_NOAEC
//#include "test19_fxp8_GDFTNoModSeq256-120-512__permuted_arm_pq.h"
#include "test22_fxp8_GDFTNoModSeq256-120-512_16kHz_166kB__permuted_arm_pq.h"
#endif
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
extern status_t initCheckDefinitionPathTxRxConversa( AUDIO_definitionPath_st* p_definitionPathTx, AUDIO_definitionPath_st* p_definitionPathRx,AUDIO_conversa_st* p_definitionConversa);
extern status_t initCheckDefinitionParameterFileConversa( AUDIO_conversa_st* p_definitionConversa);

/*******************************************************************************
 * Variables
 ******************************************************************************/
// This address will be used to shared data between each core
PL_UINT32  g_sharedAddress = CLEAR_VALUE;// Need to be initialize at 0xFFFFFFFF

/*******************************************************************************
 * Code
 ******************************************************************************/
/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            initSetConversa				   											 */
/*                                                                                               */
/* DESCRIPTION: set conversa configuration								  	 					 */
/*              create conversa instance 														 */
/*                                                                                               */
/* PARAMETERS:                                                                                   */
/*************************************************************************************************/
status_t initSetConversa(void *PtrCommonVarSharedByCore0AndCore1)
{

	/***********************/
	/* Variable Definition */
	T_CommonVarSharedByCore0AndCore1*       p_CommonVarShared;
	status_t								retStatus;
	RdspStatus 								retStatusRdsp 	 			= OK;
	void* 									ConversaHeapMemory;
	// conversa version
	PL_UINT32 								conversaVersionMajor;
	PL_UINT32 								conversaVersionMinor;
	PL_UINT32 								conversaVersionPatch;
	AUDIO_configParam_st*   				p_configParamTx;
	AUDIO_configParam_st*   				p_configParamRx;
	AUDIO_conversa_st* 						p_definitionConversa;

	if (PtrCommonVarSharedByCore0AndCore1 == NULL)
	{
		return kStatus_NullPointer;
	}
	/**************************************************/
	/* Check that we received Shared structure address*/
	if (CORE0_checkSharedStrucrAddress() == 0)
	{
		PRINTF("Shared Structure Address not receive\r\n");
		//  NEXT IMPROVEMENT We should send a mailbox response to core1
	}
	else
	{
		/* get the shared structure */
		p_CommonVarShared=(T_CommonVarSharedByCore0AndCore1 *)(PtrCommonVarSharedByCore0AndCore1);
	}
	p_configParamTx       = &p_CommonVarShared->audioDefinition.audioTxPath_handle.configParam;
	p_configParamRx       = &p_CommonVarShared->audioDefinition.audioRxPath_handle.configParam;
	p_definitionConversa  = &p_CommonVarShared->audioDefinition.swIpConversa_handle;
	/* Initialize Buffer selector */
	p_CommonVarShared->buffToChoose =0;
	/* Initialize State variable */
	p_CommonVarShared->audioDefinition.swIpConversa_handle.Core0Busy = PL_FALSE;

	/**************************************************************/
	/* Check if Conversa is compatible with Tx\Rx path parameters */
	retStatus =  initCheckDefinitionPathTxRxConversa(&p_CommonVarShared->audioDefinition.audioTxPath_handle,
													 &p_CommonVarShared->audioDefinition.audioRxPath_handle,
													 p_definitionConversa);
	if (retStatus != kStatus_Success )
	{
		return retStatus;
	}
	/**************************************************************/
	/* Check if Conversa is compatible with tuning parameters */
	if (p_CommonVarShared->audioDefinition.swIpConversa_handle.conversaTuningParamStruct.isThereTuningFile == PL_TRUE)
	{
		retStatus = initCheckDefinitionParameterFileConversa( &p_CommonVarShared->audioDefinition.swIpConversa_handle );
		if (retStatus != kStatus_Success)
		{
			PRINTF("FAIL: Conversa parameter files & Conversa library constant are not compatible\r\n");
			return kStatus_NotCompatible;
		}
		else
		{
			PRINTF("\t -Check: Compatible with current Conversa library constant\r\n");
		}
	}
	PRINTF("**********************************\r\n");
	PRINTF("Conversa : Create & Init\r\n");
	PRINTF("**********************************\r\n");
	/***************************/
	/* SET CONVERSA PARAMETERS */
	/* set conversa parameters */
#if CONVERSAML_NOAEC
	p_definitionConversa->conversaPluginParams.num_rx_in 		   				= 0;								 // Conversa Rx input channel number  is always 1. (rx_in = 1 for calling operation, rx_in = 2 for gaming headset operation)
	p_definitionConversa->conversaPluginParams.num_spks 		   				= 0;  							     // Conversa Rx output channel number is always 1. (num_spk = 1 for when having mono output, num_spk = 2 when using internal split filter (woofer and tweeter))
	p_definitionConversa->conversaPluginParams.num_mics 		   				= 1;    							// One mic solution
#else
	p_definitionConversa->conversaPluginParams.num_rx_in 		   				= 1;								 // Conversa Rx input channel number  is always 1. (rx_in = 1 for calling operation, rx_in = 2 for gaming headset operation)
	p_definitionConversa->conversaPluginParams.num_spks 		   				= 1;  							     // Conversa Rx output channel number is always 1. (num_spk = 1 for when having mono output, num_spk = 2 when using internal split filter (woofer and tweeter))
	p_definitionConversa->conversaPluginParams.num_mics 		   				= p_configParamTx->channelNumber;    // Conversa Tx input channel number.
#endif
	/* set conversa config parameters */
	p_definitionConversa->conversaPluginParams.config.aec_uses_current_sensing 	= CONVERSA_DISABLE;						// Conversa AEC reference input channel type: (1)current sensing or (0)other
	if ( p_configParamTx->channelNumber > 1)																			// if only 1 mic then no DOA possible
	{
		p_definitionConversa->conversaPluginParams.config.create_doa 			= CONVERSA_DISABLE;						// Conversa  DOA ALWAYS Disable
	}
	else
	{
		p_definitionConversa->conversaPluginParams.config.create_doa 			= CONVERSA_DISABLE;						// Conversa DOA disable (direction of arrival) process
	}
#if CONVERSAML_NOAEC
	p_definitionConversa->conversaPluginParams.config.bf_num_mics 				= 1;//p_configParamTx->channelNumber;		// beam former input data stream
#else
	p_definitionConversa->conversaPluginParams.config.bf_num_mics 				= p_configParamTx->channelNumber;		// beam former input data stream
#endif
	// if more than 1 input then we use beam former
	if (p_definitionConversa->conversaPluginParams.config.bf_num_mics > 1)
	{
		p_definitionConversa->conversaPluginParams.config.num_bf 				= CONVERSA_ENABLE;						// beam former enable
	}
	else
	{
		p_definitionConversa->conversaPluginParams.config.num_bf 				= CONVERSA_DISABLE;						// beam former => disable
	}
#if CONVERSAML_NOAEC
	p_definitionConversa->conversaPluginParams.config.aec_num_mics 				= 0;  									// No AEC
	p_definitionConversa->conversaPluginParams.config.aec_num_refs 				= CONVERSA_DISABLE;  					// No AEC
#else
	p_definitionConversa->conversaPluginParams.config.aec_num_mics 				= p_configParamTx->channelNumber;  								// Conversa AEC input channel number
	p_definitionConversa->conversaPluginParams.config.aec_num_refs 				= p_definitionConversa->conversaPluginParams.num_rx_in;  		// Conversa AEC reference input channel number
#endif
	// if aec (audio echo canceller) reference input is > 0 then we use the aec
	if (p_definitionConversa->conversaPluginParams.config.aec_num_refs > 0)
	{
		p_definitionConversa->conversaPluginParams.config.num_aec 				= CONVERSA_ENABLE;				   		// Conversa AEC enable
	}
	else
	{
		p_definitionConversa->conversaPluginParams.config.num_aec 				= CONVERSA_DISABLE;				   		// Conversa AEC disable
	}

#if APP_PLATFORM == APP_PL_LPC55S69EVK
	p_definitionConversa->conversaPluginParams.config.device_id 				= Device_LPC55S69;					   // Conversa library made for LPC55S69 EVK
#else
	#error "Frame work error, platform not supported"
#endif
#if CONVERSAML_NOAEC
	p_definitionConversa->conversaPluginParams.config.se_model_blob_is_already_open=1;
	p_definitionConversa->conversaPluginParams.SE_model_ptr = (uint8_t*)model_blob_header;
	p_definitionConversa->conversaPluginParams.SE_model_size = sizeof(model_blob_header);
#endif
	/*****************************/
	/* Allocate Heap for Conversa*/
	p_definitionConversa->conversaPluginParams.extmem_size_bytes = RdspConversa_Plugin_GetRequiredHeapMemoryBytes(&p_definitionConversa->conversaPluginParams);
	ConversaHeapMemory = malloc(p_definitionConversa->conversaPluginParams.extmem_size_bytes);
	if (ConversaHeapMemory == NULL)
	{
		return kStatus_Fail;
	}
	else
	{
		p_definitionConversa->conversaPluginParams.extmem_base_address = ConversaHeapMemory;
		PRINTF("Memory allocated for Conversa: %i bytes\r\n", p_definitionConversa->conversaPluginParams.extmem_size_bytes);
	}

	/********************************/
	/* CREATE SET CONVERSA INSTANCE */
	retStatusRdsp = RdspConversa_Plugin_Create(&p_definitionConversa->conversaPluginParams);
	if (retStatusRdsp != OK)
	{
		PRINTF("\tFAIL: Conversa: failed to create instance (error code = %i)\r\n", retStatusRdsp);
		return kStatus_Fail;
	}

	/************************/
	/*  CONVERSA INIT    	*/
	retStatusRdsp = RdspConversa_Plugin_Init(&p_definitionConversa->conversaPluginParams);
	if (retStatusRdsp != OK)
	{
		PRINTF("\tFAIL: Conversa failed to init (error code = %i)\r\n", retStatusRdsp);
		return kStatus_Fail;
	}
	/*****************************/
	/* GET CONVERSA DATA ADDRESS */
	/* Not working */
	p_definitionConversa->conversaDataControlAddress =(PL_UINT32) RdspConversa_Plugin_GetControlDataAddress( &p_definitionConversa->conversaPluginParams );
	PRINTF("Conversa control data address ( to be used for the tuning tool 'Control Addr. (hex)' ) = 0x%x\r\n",LPC_CONTROL_ADDR);
	/******************************/
	/* DISPLAY CONVERSA VERSION   */
	RdspConversa_Plugin_GetLibVersion(	&p_definitionConversa->conversaPluginParams,
										(uint32_t*) &conversaVersionMajor,
										(uint32_t*) &conversaVersionMinor,
										(uint32_t*) &conversaVersionPatch);
	PRINTF("Conversa version: %i.%i.%i \r\n",conversaVersionMajor,conversaVersionMinor,conversaVersionPatch);
	/******************************/
	/* UPDATE CONVERSA PARAMETERS */
	// Set RX volume (provided by host/QCC) [0:15] => {-inf, -42, -39, -36, ..., 0} dB gain
	RdspConversa_Plugin_SetVolume( &p_definitionConversa->conversaPluginParams,
								   15);

	/*********************************************************/
	/* Load conversa parameters from .src configuration file */
	if (p_definitionConversa->conversaTuningParamStruct.isThereTuningFile  != PL_FALSE )
	{
		// check for NULL pointer
		if (p_definitionConversa->conversaTuningParamStruct.p_conversaTuningParameter == NULL)
		{
			return kStatus_NullPointer;
		}
		else
		{
			RdspConversa_Plugin_SetParameters(	&p_definitionConversa->conversaPluginParams,
											 (const PL_UINT8*)  p_definitionConversa->conversaTuningParamStruct.p_conversaTuningParameter->info_str,	   	// string information associated to the tuning file
											 sizeof(p_definitionConversa->conversaTuningParamStruct.p_conversaTuningParameter->info_str),	   				// size of the string information
											 (const uint32_t*) &p_definitionConversa->conversaTuningParamStruct.p_conversaTuningParameter->parameter_data,	// tuning parameters to be setted
											 p_definitionConversa->conversaTuningParamStruct.p_conversaTuningParameter->parameter_data_size);				// size of the tuning parameters in PL_UINT32
		}
	}
	else
	{
		PRINTF("\tConversa set parameters: keep default one\r\n");
	}
	return kStatus_Success ;
}
/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            swProcessConversa			   										 	 */
/*                                                                                               */
/* DESCRIPTION: process input audio data by conversa 			  	 					 		 */
/*                                                                                               */
/* PARAMETERS:                                                                                   */
/*         Shared Structure address                              	 							 */
/*************************************************************************************************/
__attribute__((__section__("CodeQuickAccessInCodeRam")))
status_t swProcessConversa(void *PtrCommonVarSharedByCore0AndCore1)
{
	/***********************/
	/* Variable Definition */
	rdsp_float **    				  pp_ConversaRxOut;
	rdsp_float *     				  p_ConversaTxOut;
	rdsp_float *    				  pp_ConversaRxIn[2];
	rdsp_float *     				  pp_ConversaTxIn[2];
	rdsp_float *	 				  p_Src;
	rdsp_float *     				  p_Dst;
	rdsp_float *     				  p_Dst1;
	T_CommonVarSharedByCore0AndCore1* p_CommonVarShared;
	PL_UINT16						  framsizeDividebBy8;
	RdspStatus 						  conversaRetStatus;


	if (PtrCommonVarSharedByCore0AndCore1 == NULL)
	{
		return kStatus_NullPointer;
	}
	p_CommonVarShared=(T_CommonVarSharedByCore0AndCore1 *)PtrCommonVarSharedByCore0AndCore1;
	framsizeDividebBy8 = p_CommonVarShared->audioDefinition.audioRxPath_handle.configParam.samplePerFrame/8;
    /* Assign Process buffers */
#if RX_PATH_PRESENT
	pp_ConversaRxIn[0]=(rdsp_float *)p_CommonVarShared->pp_ConversaProcessBuff_RxIn_Flt[0];
#endif
	pp_ConversaTxIn[0]=(rdsp_float *)p_CommonVarShared->pp_ConversaProcessBuff_TxIn_Flt[0];
	pp_ConversaTxIn[1]=(rdsp_float *)p_CommonVarShared->pp_ConversaProcessBuff_TxIn_Flt[1];

	/*****************************************************************************************************************/
	/* Convert PL_INT32 buffers to Float buffers before Conversa Processing                                          */
	for (PL_INT16 iChannel=0; iChannel< p_CommonVarShared->audioDefinition.swIpConversa_handle.conversaPluginParams.num_mics; iChannel++)
	{

		arm_q31_to_float (	(const q31_t*) 	  p_CommonVarShared->pp_ConversaProcessBuff_TxIn[1-p_CommonVarShared->buffToChoose][iChannel],
											  pp_ConversaTxIn[iChannel],
											  p_CommonVarShared->audioDefinition.audioRxPath_handle.configParam.samplePerFrame
						 );
	}
#if RX_PATH_PRESENT
	/* Rx convert reference data Q31 to float */
	for (PL_INT16 iChannel=0; iChannel<p_CommonVarShared->audioDefinition.swIpConversa_handle.conversaPluginParams.num_rx_in; iChannel++)
	{
		arm_q31_to_float (	(const q31_t*) 	p_CommonVarShared->pp_ConversaProcessBuff_RxIn[1-p_CommonVarShared->buffToChoose][iChannel],
											pp_ConversaRxIn[iChannel],
											p_CommonVarShared->audioDefinition.audioRxPath_handle.configParam.samplePerFrame
						 );
	}
#endif
	/*******************************************************************/
	/* Conversa Process Call                                           */
	/* NEXT IMPROVEMENT :  call directly Q31 Conversa Process function */
	conversaRetStatus = RdspConversa_Plugin_Process(&p_CommonVarShared->audioDefinition.swIpConversa_handle.conversaPluginParams,
													pp_ConversaTxIn,
													pp_ConversaRxIn,
													NULL );
	if (conversaRetStatus == LICENSE_EXPIRED)
	{
		// Timeout append
		return kStatus_LicenseError;
	}
	if (conversaRetStatus != OK)
	{
		PRINTF("[swProcess] FAIL: RdspConversa_Plugin_Process failed with error code: %i\r\n", conversaRetStatus);
		return kStatus_Fail;
	}
	// Get Conversa output pointers
#if RX_PATH_PRESENT
	pp_ConversaRxOut = RdspConversa_Plugin_GetRxOut(&p_CommonVarShared->audioDefinition.swIpConversa_handle.conversaPluginParams);
#endif
	p_ConversaTxOut  = RdspConversa_Plugin_GetTxOut(&p_CommonVarShared->audioDefinition.swIpConversa_handle.conversaPluginParams);
	/*****************************************************************************************************************/
	/* Convert Float  buffers to PL_INT32 buffers after Conversa Processing                                          */
	arm_float_to_q31 (	(const PL_FLOAT*) 	p_ConversaTxOut,
						(q31_t*) 			p_CommonVarShared->pp_ConversaProcessBuff_TxOut[1-p_CommonVarShared->buffToChoose][0],
											p_CommonVarShared->audioDefinition.audioRxPath_handle.configParam.samplePerFrame
					 );
	// convert output ConversaRx data float to Q31
#if RX_PATH_PRESENT
	arm_float_to_q31 (	(const PL_FLOAT*) 	pp_ConversaRxOut[0],   	// channel 0 Rx
						(q31_t*) 			p_CommonVarShared->pp_ConversaProcessBuff_RxOut[1-p_CommonVarShared->buffToChoose][0],
											p_CommonVarShared->audioDefinition.audioRxPath_handle.configParam.samplePerFrame
					  );
#endif
	 // This flag indicate that Core0 is not busy anymore
	p_CommonVarShared->audioDefinition.swIpConversa_handle.Core0Busy = PL_FALSE;
	return kStatus_Success ;
}
