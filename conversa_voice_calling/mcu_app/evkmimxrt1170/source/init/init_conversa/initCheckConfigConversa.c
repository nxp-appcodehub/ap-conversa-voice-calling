/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// general
#include "appGlobal.h"

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

/*******************************************************************************
 * Code
 ******************************************************************************/
/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            initCheckDefinitionParameterFileConversa			 				     */
/*                                                                                               */
/* DESCRIPTION: Check Conversa configuration is compliant with Conversa library constant	 	 */
/*                                                                                               */
/* PARAMETERS:                                                                                   */
/*************************************************************************************************/

status_t initCheckDefinitionParameterFileConversa( AUDIO_conversa_st* p_definitionConversa)
{
	status_t retStatus 		 = kStatus_Success;
	status_t retStatusFunc	 = kStatus_Success;

	rdsp_conversa_plugin_constants_t* p_conversaPluginConst     = &p_definitionConversa->conversaPluginConst;
	conversa_parameter_config_t*      p_conversaTuningParameter = p_definitionConversa->conversaTuningParamStruct.p_conversaTuningParameter;

	/***************************/
	/* CHECK PARAMETER POINTER */
	if ((p_definitionConversa == NULL))
	{
		return kStatus_NullPointer;
	}
	else
	{
		if (p_definitionConversa->conversaTuningParamStruct.p_conversaTuningParameter == NULL)
		{
			return kStatus_NullPointer;
		}
	}

	// Read conversa library limitation
	RdspConversa_Plugin_GetConstants(p_conversaPluginConst);

	/***********************************************************************/
	/* CHECK GENERAL PARAMETER MATCH WITH CONVERSA LIB LIMITATION */

	// TODO check conversa_lib_version
	// TODO	check settings_format_version

	// Check audio frame size
	if ( p_conversaTuningParameter->dsp_blocksize != p_conversaPluginConst->audio_framesize)
	{
		PRINTF("FAIL: Sample per frame rate => Conversa parameter file (%d) not equal to Conversa library (%d)\r\n", p_conversaTuningParameter->dsp_blocksize ,p_conversaPluginConst->audio_framesize);
		retStatus = kStatus_NotCompatible;
	}

	/***********************************************************************/
	/* CHECK PARAMETER TX MATCH WITH CONVERSA LIB LIMITATION */

	// Check frame rate
	if ( p_conversaTuningParameter->tx_samplerate != p_conversaPluginConst->tx_fs)
	{
		PRINTF("FAIL: Sample rate => Conversa parameter file (%d) not equal to Conversa library tx (%d)\r\n", p_conversaTuningParameter->tx_samplerate, p_conversaPluginConst->tx_fs);
		retStatus = kStatus_NotCompatible;
	}

	// Check maximal Tx path number of channel (microphones)
	if ( p_conversaTuningParameter->num_mic > p_conversaPluginConst->max_num_mics)
	{
		PRINTF("FAIL: number of channel => Conversa parameter file (%d) > Conversa library (%d)\r\n", p_conversaTuningParameter->num_mic ,p_conversaPluginConst->max_num_mics);
		retStatus = kStatus_NotCompatible;
	}

	/***********************************************************************/
	/* CHECK PARAMETER RX MATCH WITH CONVERSA LIB LIMITATION */

	// Check frame rate
	if ( p_conversaTuningParameter->rx_samplerate != p_conversaPluginConst->rx_fs)
	{
		PRINTF("FAIL: Sample rate => Conversa parameter file (%d) not equal to Conversa library tx (%d)\r\n", p_conversaTuningParameter->rx_samplerate, p_conversaPluginConst->rx_fs);
		retStatus = kStatus_NotCompatible;
	}

	return retStatus;
}

/**************************************************************************************************/
/*                                                                                                */
/* FUNCTION:            initCheckDefinitionPathTxRxConversa										  */
/*                                                                                                */
/* DESCRIPTION: Check all conversa & path Tx\Rx parameter are compatible together				  */
/*                                                                                                */
/* PARAMETERS:                                                                                    */
/**************************************************************************************************/

status_t initCheckDefinitionPathTxRxConversa( AUDIO_definitionPath_st* p_definitionPathTx,
											  AUDIO_definitionPath_st* p_definitionPathRx,
											  AUDIO_conversa_st*	   p_definitionConversa
										    )
{

	status_t retStatus 		 = kStatus_Success;
	status_t retStatusFunc	 = kStatus_Success;

	rdsp_conversa_plugin_constants_t* p_conversaPluginConst = &p_definitionConversa->conversaPluginConst;

	/***************************/
	/* CHECK PARAMETER POINTER */
	if ((p_definitionPathTx == NULL) || (p_definitionPathRx == NULL) || (p_definitionConversa == NULL))
	{
		return kStatus_NullPointer;
	}

	// Read conversa library limitation
	RdspConversa_Plugin_GetConstants(p_conversaPluginConst);

	/***********************************************************************/
	/* CHECK PARAMETER INTERNAL PATH TX MATCH WITH CONVERSA LIB LIMITATION */

	// Check frame rate
	if ( p_definitionPathTx->configParam.sampleRate != p_conversaPluginConst->tx_fs)
	{
		PRINTF("FAIL: Sample rate => Tx Path (%d) not equal to Conversa library tx (%d)\r\n", p_definitionPathTx->configParam.sampleRate, p_conversaPluginConst->tx_fs);
		retStatus = kStatus_NotCompatible;
	}

	// Check audio frame size
	if ( p_definitionPathTx->configParam.samplePerFrame != p_conversaPluginConst->audio_framesize)
	{
		PRINTF("FAIL: Sample per frame rate => Tx Path (%d) not equal to Conversa library (%d)\r\n", p_definitionPathTx->configParam.samplePerFrame ,p_conversaPluginConst->audio_framesize);
		retStatus = kStatus_NotCompatible;
	}

	// Check maximal Tx path number of channel (microphones)
	if ( p_definitionPathTx->configParam.channelNumber > p_conversaPluginConst->max_num_mics)
	{
		PRINTF("FAIL: number of channel => Tx Path (%d) > Conversa library (%d)\r\n", p_definitionPathTx->configParam.channelNumber ,p_conversaPluginConst->max_num_mics);
		retStatus = kStatus_NotCompatible;
	}

	// Check maximal bytes per sample
	if ( (p_definitionPathTx->configParam.bitPerSample/8) != p_conversaPluginConst->num_bytes_per_sample)
	{
		PRINTF("FAIL: sample depth => Tx Path (%d) not equal to Conversa library (%d)\r\n", p_definitionPathTx->configParam.bitPerSample/8 ,(p_conversaPluginConst->num_bytes_per_sample));
		retStatus = kStatus_NotCompatible;
	}

	/***********************************************************************/
	/* CHECK PARAMETER INTERNAL PATH RX MATCH WITH CONVERSA LIB LIMITATION */

	// Check frame rate
	if ( p_definitionPathRx->configParam.sampleRate != p_conversaPluginConst->rx_fs)
	{
		PRINTF("FAIL: Sample rate => Rx Path (%d) not equal to Conversa library tx (%d)\r\n", p_definitionPathRx->configParam.sampleRate, p_conversaPluginConst->rx_fs);
		retStatus = kStatus_NotCompatible;
	}

	// Check audio frame size
	if ( p_definitionPathRx->configParam.samplePerFrame != p_conversaPluginConst->audio_framesize)
	{
		PRINTF("FAIL: Sample per frame rate => Rx Path (%d) not equal to Conversa library (%d)\r\n", p_definitionPathRx->configParam.samplePerFrame ,p_conversaPluginConst->audio_framesize);
		retStatus = kStatus_NotCompatible;
	}

	// Check maximal number of channel (speaker)
	if ( p_definitionPathRx->configParam.channelNumber > p_conversaPluginConst->max_num_spks)
	{
		PRINTF("FAIL: number of channel => Rx Path (%d) > Conversa library (%d)\r\n", p_definitionPathRx->configParam.channelNumber ,p_conversaPluginConst->max_num_spks);
		retStatus = kStatus_NotCompatible;
	}

	// Check maximal bytes per sample
	if ( (p_definitionPathRx->configParam.bitPerSample/8) != p_conversaPluginConst->num_bytes_per_sample)
	{
		PRINTF("FAIL: sample depth => Rx Path (%d) not equal to Conversa library (%d)\r\n", p_definitionPathRx->configParam.bitPerSample/8 ,(p_conversaPluginConst->num_bytes_per_sample));
		retStatus = kStatus_NotCompatible;
	}

	return retStatus;
}

#endif
