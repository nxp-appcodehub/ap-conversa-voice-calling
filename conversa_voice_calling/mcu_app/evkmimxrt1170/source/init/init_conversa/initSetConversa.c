/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// general
#include "appGlobal.h"
#include <string.h>

// audio
#include "audio.h"

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
__attribute__((__section__(".data.$RAM_CONVERSA_CONTROL_BLOCK"))) PL_UINT32 s_conversaControlBlockAddress;
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

status_t initSetConversa( 	AUDIO_configParam_st*   p_configParamTx,
							AUDIO_configParam_st*   p_configParamRx,
							AUDIO_conversa_st* 		p_definitionConversa)
{
	status_t   retStatus 		 = kStatus_Success;
	status_t   retStatusFunc	 = kStatus_Success;
	RdspStatus retStatusRdsp 	 = OK;

	// conversa version
	PL_UINT32 conversaVersionMajor;
	PL_UINT32 conversaVersionMinor;
	PL_UINT32 conversaVersionPatch;

	int errnum;
	FILE *p_tuningFile;

	PL_UINT32 conversaExtmemSizeBytesReal = 0; 		// the real memory size need by conversa in bytes, value get after hconversa instance created

	PL_UINT32 iDebug;
	PL_UINT8* p_debug8bit;

	PRINTF("\nConversa library:\r\n");

	/***************************/
	/* CHECK PARAMETER POINTER */
	if ( (p_configParamTx == NULL) || (p_configParamRx == NULL) || (p_definitionConversa == NULL))
	{
		return kStatus_NullPointer;
	}

	// enable conversa process
	p_definitionConversa->operatingModeTx = AUDIO_OM_ENABLE;        // conversa TX/RX path enabled
	p_definitionConversa->operatingModeRx = AUDIO_OM_ENABLE;        // conversa TX/RX path enabled

	/***************************/
	/* SET CONVERSA PARAMETERS */
	/* set conversa parameters */
	p_definitionConversa->conversaPluginParams.extmem_base_address = &g_conversaMemory;
	p_definitionConversa->conversaPluginParams.extmem_size_bytes   = CONVERSA_MEMORY_BYTE;
	p_definitionConversa->conversaPluginParams.num_rx_in 		   = 1;								 // Conversa Rx input channel number  is always 1. (rx_in = 1 for calling operation, rx_in = 2 for gaming headset operation)
	p_definitionConversa->conversaPluginParams.num_spks 		   = 1;  							 // Conversa Rx output channel number is always 1. (num_spk = 1 for when having mono output, num_spk = 2 when using internal split filter (woofer and tweeter))
	p_definitionConversa->conversaPluginParams.num_mics 		   = p_configParamTx->channelNumber; // Conversa Tx input channel number.

	/* set conversa config parameters */
	p_definitionConversa->conversaPluginParams.config.aec_uses_current_sensing 	= CONVERSA_DISABLE;						// Conversa AEC reference input channel type: (1)current sensing or (0)other

	if ( p_configParamTx->channelNumber > 1)																			// if only 1 mic then no DOA possible
	{
		p_definitionConversa->conversaPluginParams.config.create_doa 			= CONVERSA_ENABLE;						// Conversa DOA enable (direction of arrival) process
	}
	else
	{
		p_definitionConversa->conversaPluginParams.config.create_doa 			= CONVERSA_DISABLE;						// Conversa DOA disable (direction of arrival) process
	}
	p_definitionConversa->conversaPluginParams.config.bf_num_mics 				= p_configParamTx->channelNumber;		// beam former input data stream

	// if more than 1 input then we use beam former
	if (p_definitionConversa->conversaPluginParams.config.bf_num_mics > 1)
	{
		p_definitionConversa->conversaPluginParams.config.num_bf 				= CONVERSA_ENABLE;						// beam former enable
	}
	else
	{
		p_definitionConversa->conversaPluginParams.config.num_bf 				= CONVERSA_DISABLE;						// beam former => disable
	}

	p_definitionConversa->conversaPluginParams.config.aec_num_mics 				= p_configParamTx->channelNumber;  								// Conversa AEC input channel number
	p_definitionConversa->conversaPluginParams.config.aec_num_refs 				= p_definitionConversa->conversaPluginParams.num_rx_in;  		// Conversa AEC reference input channel number

	// if aec (audio echo canceller) reference input is > 0 then we use the aec
	if (p_definitionConversa->conversaPluginParams.config.aec_num_refs > 0)
	{
		p_definitionConversa->conversaPluginParams.config.num_aec 				= CONVERSA_ENABLE;				   		// Conversa AEC enable
	}
	else
	{
		p_definitionConversa->conversaPluginParams.config.num_aec 				= CONVERSA_DISABLE;				   		// Conversa AEC disable
	}

#if APP_PLATFORM == APP_PL_RT1170EVK
	p_definitionConversa->conversaPluginParams.config.device_id 				= Device_IMXRT1170_CM7;					// Conversa library made for I.MX RT1170 cortexM7
#else
	#error "Frame work error, platform not supported"
#endif

	/**********************************************************/
	/* CHECK MEMORY SIZE RESERVED FOR CONVERSA IS BIG ENOUGH */
	conversaExtmemSizeBytesReal = RdspConversa_Plugin_GetRequiredHeapMemoryBytes(&p_definitionConversa->conversaPluginParams);    // get Conversa required size
	// check allocate size is optimized
	if ( conversaExtmemSizeBytesReal > p_definitionConversa->conversaPluginParams.extmem_size_bytes )
	{
		PRINTF("\t-Memory allocated for Conversa: %i bytes\r\n", p_definitionConversa->conversaPluginParams.extmem_size_bytes);
		PRINTF("\t-Conversa memory size required: %i bytes\r\n", conversaExtmemSizeBytesReal);
		PRINTF("\tFAIL: Conversa memory allocation\r\n");
		return kStatus_Fail;
	}
	else if ( conversaExtmemSizeBytesReal < p_definitionConversa->conversaPluginParams.extmem_size_bytes )
	{

		PRINTF("\t-Conversa memory size required: %i bytes\r\n", conversaExtmemSizeBytesReal);
		PRINTF("\t-Memory allocated for Conversa: %i bytes\r\n", p_definitionConversa->conversaPluginParams.extmem_size_bytes);
		PRINTF("\t-Conversa memory allocation: PASS but not optimized\r\n");
	}
	else
	{
		PRINTF("\t-Memory allocated for Conversa: %i bytes\r\n", p_definitionConversa->conversaPluginParams.extmem_size_bytes);
		PRINTF("\t-Conversa memory size required: %i bytes\r\n", conversaExtmemSizeBytesReal);
		PRINTF("\t-Conversa memory allocation: PASS\r\n");
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
	/* INITIALSIE CONVERSA 	*/
	retStatusRdsp = RdspConversa_Plugin_Init(&p_definitionConversa->conversaPluginParams);
	if (retStatusRdsp != OK)
	{
		PRINTF("\tFAIL: Conversa failed to init (error code = %i)\r\n", retStatusRdsp);
		return kStatus_Fail;
	}

	/******************************/
	/* GET CONVERSA REQUIRED SIZE */
	/* 		get the reserved the memory size for conversa
	 */
	conversaExtmemSizeBytesReal = RdspConversa_Plugin_GetAllocatedMemoryBytes(&p_definitionConversa->conversaPluginParams);
	//PRINTF("\tConversa memory size required is %i bytes\r\n", conversaExtmemSizeBytesReal);

	/*****************************/
	/* GET CONVERSA DATA ADDRESS
	/*   Get the address of the tuning structure
	 */
	p_definitionConversa->conversaDataControlAddress = (PL_UINT32) RdspConversa_Plugin_GetControlDataAddress( &p_definitionConversa->conversaPluginParams );
	// store the address of the tuning structure in the reserved area
	s_conversaControlBlockAddress = p_definitionConversa->conversaDataControlAddress;

	//p_definitionConversa->conversaDataControlAddress = (PL_UINT32) RdspConversa_Plugin_GetControlDataPointer( &p_definitionConversa->conversaPluginParams );
	PRINTF("\t-Conversa control data address ( to be used for the tuning tool 'Control Addr. (hex)' ) = 0x%x\r\n", &s_conversaControlBlockAddress);


	/******************************/
	/* DISPAY CONVERSA VERSION    */
	RdspConversa_Plugin_GetLibVersion(	&g_appHandle.audioDefinition.swIpConversa_handle.conversaPluginParams,
										(uint32_t*) &conversaVersionMajor,
										(uint32_t*) &conversaVersionMinor,
										(uint32_t*) &conversaVersionPatch);

	PRINTF("\t-Conversa version: %i.%i.%i \r\n",conversaVersionMajor,conversaVersionMinor,conversaVersionPatch);

	/******************************/
	/* UPDATE CONVERSA PARAMETERS */
	// Set RX volume (provided by host/QCC) [0:15] => {-inf, -42, -39, -36, ..., 0} dB gain
	RdspConversa_Plugin_SetVolume( &p_definitionConversa->conversaPluginParams,
								   15);


	/*********************************/
	/* Load conversa parameters from .c configuration file */
	// check if configuration define else keep default configuration

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


	/* At this step the Conversa Rx and Tx path is ready to be use in process function */

	return retStatus;
}

#endif
