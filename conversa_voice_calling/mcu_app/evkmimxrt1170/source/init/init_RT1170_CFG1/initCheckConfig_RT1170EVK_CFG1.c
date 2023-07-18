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

/*******************************************************************************
 * Code
 ******************************************************************************/

/**************************************************************************************************/
/*                                                                                                */
/* FUNCTION:            initCheckDefinitionPathTx												  */
/*                                                                                                */
/* DESCRIPTION: Check all path Tx parameter are compatible together and supported by the platform */
/*                                                                                                */
/* PARAMETERS:                                                                                    */
/**************************************************************************************************/
status_t initCheckDefinitionPathTxRx(	AUDIO_definitionPath_st* p_definitionPathTx,
										AUDIO_definitionPath_st* p_definitionPathRx)
{

	status_t retStatus 		 = kStatus_Success;
	status_t retStatusFunc	 = kStatus_Success;

	/***************************/
	/* CHECK PARAMETER POINTER */
	if ( (p_definitionPathTx == NULL)
	   ||(p_definitionPathRx == NULL) )
	{
		return kStatus_NullPointer;
	}

	/*********************************/
	/* CHECK PARAMETER INTERNAL PATH */

	/* Rx source == AUDIO_SRC_USB1 => check sink */
	if ( p_definitionPathRx->configParam.source == AUDIO_SRC_USB1 )
	{
		if (p_definitionPathRx->configParam.sink == AUDIO_SINK_SPEAKER2)
		{
			PRINTF("WARNING: Rx source (AUDIO_SRC_USB1) and Rx sink (AUDIO_SINK_SPEAKER2) => same audio signal will be play on the 2nd speaker\r\n");
		}
		else if ( p_definitionPathRx->configParam.sink != AUDIO_SINK_SPEAKER1 )
		{
			PRINTF("FAIL: Rx source (AUDIO_SRC_USB1) and Rx sink are not compatible\r\n");
			return kStatus_NotCompatible;
		}
	}
	else if ( p_definitionPathRx->configParam.source == AUDIO_SRC_USB2 )
	{
		if (   ( p_definitionPathRx->configParam.sink  != AUDIO_SINK_SPEAKER2 )
			&& ( p_definitionPathRx->configParam.sink  != AUDIO_SINK_SPEAKER1DIFF )
		   )
		{
			PRINTF("FAIL: Rx source (AUDIO_SRC_USB2) and Rx sink are not compatible\r\n");
			return kStatus_NotCompatible;
		}
	}

	/* Rx Tx sample rate */
	if  (p_definitionPathRx->configParam.sampleRate != p_definitionPathTx->configParam.sampleRate)
	{
		PRINTF("FAIL: Rx sample rate (%d) and Tx sample rate are not compatible (%d)\r\n",p_definitionPathRx->configParam.sampleRate,p_definitionPathTx->configParam.sampleRate);
		return kStatus_NotCompatible;
	}

	/* Rx Tx sample per frame */
	if  (p_definitionPathRx->configParam.samplePerFrame != p_definitionPathTx->configParam.samplePerFrame)
	{
		PRINTF("FAIL: Rx sample per frame (%d) and Tx sample per frame are not compatible (%d)\r\n",p_definitionPathRx->configParam.samplePerFrame,p_definitionPathTx->configParam.samplePerFrame);
		return kStatus_NotCompatible;
	}

	/* Rx Tx bit per sample */
	if  (p_definitionPathRx->configParam.bitPerSample != p_definitionPathTx->configParam.bitPerSample)
	{
		PRINTF("FAIL: Rx bit per sample (%d) and Tx bit per sample are not compatible (%d)\r\n",p_definitionPathRx->configParam.bitPerSample,p_definitionPathTx->configParam.bitPerSample);
		return kStatus_NotCompatible;
	}

	return retStatus;
}
#endif
