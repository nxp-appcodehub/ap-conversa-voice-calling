/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Board includes */
// general
#include "appGlobal.h"

// board
#include "board.h"

// codec
#include "fsl_codec_common.h"
#include "fsl_wm8960.h"
#include "fsl_codec_adapter.h"

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
/*
 *  Audio codec initial configuration
 *      - It will be update after user shell command
 */
wm8960_config_t s_wm8960Config = {
									.i2cConfig        = {.codecI2CInstance = 5, .codecI2CSourceClock = 24000000U},
									.route            = kWM8960_RoutePlaybackandRecord,
									.leftInputSource  = kWM8960_InputDifferentialMicInput3,
									.rightInputSource = kWM8960_InputDifferentialMicInput2,
									.playSource       = kWM8960_PlaySourceDAC,
									.slaveAddress     = WM8960_I2C_ADDR,
									.bus              = kWM8960_BusI2S,
									.format           = { .mclk_HZ = APP_AUDIO_SAI_CLK_FREQ,			  // SAI bit clock
														  .sampleRate = kWM8960_AudioSampleRate16KHz,     // SAI sample rate default
														  .bitWidth = kWM8960_AudioBitWidth32bit},        // SAI bit width default
									.master_slave = false, 												  // true is master, false is slave
								};

codec_config_t s_boardCodecConfig = {.codecDevType = kCODEC_WM8960, .codecDevConfig = &s_wm8960Config};

/*****************************************************************************************
 * Function
 *****************************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

/***********************************************************
 *
 *  INIT audio codec
 *
 ***********************************************************/
status_t initAudioCodec( AUDIO_definitionPath_st* p_audioTxPath_handle,
						 AUDIO_definitionPath_st* p_audioRxPath_handle)
{
	status_t returnStatus;
	status_t returnStatusFunc;

    // Codec handle
    codec_handle_t	audioCodecHandle;

	/*
	 * Update codec initial parameter with path configuration
	 */
    if(    		( p_audioRxPath_handle->configParam.sink == AUDIO_SINK_SPEAKER1 )
    		||  ( p_audioRxPath_handle->configParam.sink == AUDIO_SINK_SPEAKER1DIFF)
    		|| 	( p_audioRxPath_handle->configParam.sink == AUDIO_SINK_SPEAKER2 )
	   )
    {
		s_wm8960Config.format.sampleRate = p_audioRxPath_handle->configParam.sampleRate;			// update sample rate
		s_wm8960Config.format.bitWidth   = p_audioRxPath_handle->configParam.bitPerSample;          // update bit per sample
    }
    else
    {
    	return kStatus_OutOfRange;
    }

	/* Use default setting to init codec */
	returnStatusFunc = CODEC_Init( 	&audioCodecHandle,
			 	 	 	 	 	 	&s_boardCodecConfig);
	if ( returnStatusFunc != kStatus_Success)
	{
		return returnStatusFunc;
	}

	/* Set Rx volume*/
	if ( g_appHandle.platformAndMaterials == APP_PLATFORM_MATERIALS_RT1170 )
	{
		CODEC_SetVolume(&audioCodecHandle,
						kCODEC_PlayChannelHeadphoneLeft | kCODEC_PlayChannelHeadphoneRight,
						APP_AUDIO_CODEC_VOLUME_GAIN_6DB);
	}
	else if ( g_appHandle.platformAndMaterials == 	APP_PLATFORM_MATERIALS_RT1170_TB2136 )
	{
		CODEC_SetVolume(&audioCodecHandle,
						kCODEC_PlayChannelHeadphoneLeft | kCODEC_PlayChannelHeadphoneRight,
						APP_AUDIO_CODEC_VOLUME_GAIN_6DB);
	}
	else if ( g_appHandle.platformAndMaterials == 		APP_PLATFORM_MATERIALS_RT1170_SHB_AMP2 )
	{
		CODEC_SetVolume(&audioCodecHandle,
						kCODEC_PlayChannelHeadphoneLeft | kCODEC_PlayChannelHeadphoneRight,
						APP_AUDIO_CODEC_VOLUME_GAIN_2DB);
	}
	else
	{
		PRINTF("FAIL: Rx path CODEC configure platform not defined.\r\n");
		return kStatus_Fail;
	}

	if ( returnStatusFunc != kStatus_Success )
	{
		return returnStatusFunc;
	}

    return kStatus_Success;
}

#endif
