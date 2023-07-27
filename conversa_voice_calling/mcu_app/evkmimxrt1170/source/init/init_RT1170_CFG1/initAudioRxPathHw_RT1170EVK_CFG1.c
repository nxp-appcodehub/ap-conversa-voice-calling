/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>

// General
#include "appGlobal.h"

// init
#include "init.h"

// debug
#include "fsl_debug_console.h"

// SAI
#include "fsl_sai_edma.h"
#include "fsl_codec_common.h"

// Codec
#include "fsl_wm8960.h"
#include "fsl_codec_adapter.h"

// EDMA
#include "fsl_dmamux.h"

// IRQ & callback
#include "IRQ_callBack.h"

#if  (APP_PLATFORM == APP_PL_RT1170EVK)

#define CODEC_OUT_CHANNEL_NUMBER_ALWAYS_2 	2 	// codec output channel number is always 2

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*****************************************************************************************
 * Function
 *****************************************************************************************/
status_t INIT_audioRxPathclock	(AUDIO_definition_st* 	 p_definition);
status_t INIT_audioRxPathSource (AUDIO_definition_st* 	 p_definition);
status_t INIT_audioRxPathSink	(AUDIO_definition_st* 	 p_definition);

/*******************************************************************************
 * Code
 ******************************************************************************/

/***********************************************************
 *
 *  INIT audio Rx path source for RT1170EVK_REVA
 *
 ***********************************************************/
status_t initAudioRxPathHw( AUDIO_definition_st* p_definition )
{
	// Error variables
	status_t retStatusFunc = kStatus_Success;
	status_t retStatus 	   = kStatus_Success;

    retStatusFunc = INIT_audioRxPathclock( p_definition );
	if (retStatusFunc != kStatus_Success)
	{
		PRINTF("FAIL: fail to init audio Rx clock for RT1170EVK_REVA \r\n");
		retStatus = kStatus_Fail;
	}

    retStatusFunc = INIT_audioRxPathSink( p_definition );
	if (retStatusFunc != kStatus_Success)
	{
		PRINTF("FAIL: fail to init audio Rx sink for RT1170EVK_REVA \r\n");
		retStatus = kStatus_Fail;
	}

    return retStatus;
}

/***********************************************************
 *
 *  INIT audio Rx path source for RT1170EVK_REVA
 *
 ***********************************************************/
status_t INIT_audioRxPathclock( AUDIO_definition_st*  p_definition )
{
	status_t retStatus 		 = kStatus_Success;
	status_t retStatusFunc	 = kStatus_Success;

    PL_UINT8 saiClockDiv;  // sai clock divider value

	AUDIO_definitionPath_st* p_definitionPath 	   = &p_definition->audioRxPath_handle;
	AUDIO_configParam_st* 	 p_configParam    	   = &p_definitionPath->configParam;

	/************************************************************************************
	 * Clock init
	 */
    if  (   	(p_configParam->sink == AUDIO_SINK_SPEAKER1)
    		||  (p_configParam->sink == AUDIO_SINK_SPEAKER1DIFF)
    		||  (p_configParam->sink == AUDIO_SINK_SPEAKER2)
		)
    {
    	/* Clock setting for LPI2C connected to the wm8960Config */
    	CLOCK_SetRootClockMux(kCLOCK_Root_Lpi2c5, 1);

		/*
		 * Clock root setting for SAI1 mic
		 * 			Select audio PLL clock as input (reminder: AUDIO PLL clock = 393.24 MHz)
		 * 			Configure the SAI1 mic root clock to 24.576 MHz
		 */
		CLOCK_SetRootClockMux(kCLOCK_Root_Sai1, 4);     // SAI clock input module is Audio Pll clock => 393.24 MHz
		if (p_configParam->channelNumber == 1)
		{
			saiClockDiv = 8;
		}
		else if (p_configParam->channelNumber == 2)
		{
			saiClockDiv = 16;
		}
		CLOCK_SetRootClockDiv(kCLOCK_Root_Sai1, saiClockDiv);    // 1 channel SAI clock input is divide by 8 => 393.24 MHz / 8 = 49.155 MHz
																 // 2 channel SAI clock input is divide by 16 => 393.24 MHz / 16 = 24.576 MHz

		/*Enable SAI MCLK clock*/
		retStatusFunc = initBoarClockEnableSaiMclkOutput(true);
		if( retStatusFunc != kStatus_Success )
		{
			return kStatus_Fail;
		}
    }
	else
	{
		PRINTF("FAIL: audio Rx source not supported (initAudioRxPathHw / clock init) \r\n");
		return kStatus_OutOfRange;
	}

    /* end clock init */

	return retStatus;
}

/***********************************************************
 *
 *  INIT audio Rx path source for RT1170EVK_REVA
 *
 ***********************************************************/
status_t INIT_audioRxPathSink( AUDIO_definition_st* 	p_definition )
{
	status_t retStatus 		 = kStatus_Success;
	status_t retStatusFunc	 = kStatus_Success;

	AUDIO_definitionPath_st* p_definitionPath 	   = &p_definition->audioRxPath_handle;
	AUDIO_configParam_st* 	 p_configParam    	   = &p_definitionPath->configParam;
	AUDIO_workFlowHandle_st* p_audioWorkFlow_handle= &p_definition->audioWorkFlow_handle;

	// SAI variables
	sai_transceiver_t 	saiConfig;					// sai config
    sai_word_width_t 	saiBitWidth;				// sai bit width struct
    sai_mono_stereo_t 	saiMode;					// sai mode struct monoL monoR stereo

    // DMA MUX init already done

    // DMA CONFIG already done

    /*
     * If Rx use speaker then configure the SAI
     */
    if  (   (p_configParam->sink == AUDIO_SINK_SPEAKER1)
    	 || (p_configParam->sink == AUDIO_SINK_SPEAKER1DIFF)
    	 || (p_configParam->sink == AUDIO_SINK_SPEAKER2)
		)
    {
    	/*
    	 * SAI Tx + EDMA INIT
    	 * 		- SAI tx is master ans send data to the the codec
    	 * 		- data are transfered by EDMA
    	 *
    	 * 	  => Audio task main loop will create DMA transfer with ping pong buffer mechanism
    	 */

    	/*
    	 * DMA INIT for sai Tx
    	 *   - set SAI Tx dma channel
    	 *   - set SAI Tx dma request
    	 *   - create sai tx handle
		 */
    	// set set SAI Tx dma request for the SAI Tx dma channel
    	DMAMUX_SetSource( APP_AUDIO_DMA_DMAMUX,
						  APP_AUDIO_EDMA_CHANNEL_SAI1TX,
						  (uint8_t) APP_AUDIO_EDMA_REQUEST_SOURCE_SAI1TX);

    	// enable the SAI Tx dma channel
		DMAMUX_EnableChannel( APP_AUDIO_DMA_DMAMUX,
							  APP_AUDIO_EDMA_CHANNEL_SAI1TX);

		// create sai tx handle
		EDMA_CreateHandle(	&g_dmaSaiTxHandle,
							APP_AUDIO_DMA_BASE_ADDRESS,
							APP_AUDIO_EDMA_CHANNEL_SAI1TX);

		/*
		 * Set DMA IRQ priority
		 */
		NVIC_SetPriority(DMA0_DMA16_IRQn,
				         IRQ_PRIORITY_RX_SAI_DMA);

		#if defined(FSL_FEATURE_EDMA_HAS_CHANNEL_MUX) && FSL_FEATURE_EDMA_HAS_CHANNEL_MUX
			EDMA_SetChannelMux(DEMO_DMA, DEMO_TX_EDMA_CHANNEL, APP_AUDIO_SAI_TX_EDMA_CHANNEL);
		#endif

    	// if  (p_configParam->channelNumber == 1)  { saiMode = kSAI_MonoRight;  } // not used because SAI Tx is used as I2S so 2 channel minimum are required
		if (p_configParam->channelNumber == 2)  { saiMode = kSAI_Stereo;	  }
    	else									     { return kStatus_OutOfRange; }

    	if      (p_configParam->bitPerSample == 16)  { saiBitWidth = kSAI_WordWidth16bits;	}
    	else if (p_configParam->bitPerSample == 32)  { saiBitWidth = kSAI_WordWidth32bits;	}
    	else										 { return kStatus_OutOfRange;			}

    	// SAI Init
    	SAI_Init(APP_AUDIO_SAI);

		/* Set SAI TX
    	 * 		Set SAI parameters:
    	 * 			- saiMode (mono or stereo)
    	 *      	- saiBitWidth (16 or 32 bit per sample)
    	 *      configure Sai & Edma
 		 */
		SAI_TransferTxCreateHandleEDMA( APP_AUDIO_SAI,
										&g_saiTxEdmaHandle,			// sai Tx dma handle
										callBack_saiTx,				// SAI Tx call back
										p_definition,				// call back parameter
										&g_dmaSaiTxHandle);			// dma Sai tx handle

		//SAI_TransferTxCreateHandleEDMA(DEMO_SAI, &txHandle, callback, NULL, &g_dmaHandle);

		// get default sai configuration for I2S purpose
		SAI_GetClassicI2SConfig( &saiConfig,
								 saiBitWidth,
								 saiMode,
								 1U << APP_AUDIO_SAI_CHANNEL);

		// update sai configuration to match requirements
		saiConfig.bitClock.bclkSource   = kSAI_BclkSourceMclkDiv;
		saiConfig.masterSlave           = kSAI_Master;

		// configure the sai tx
		SAI_TransferTxSetConfigEDMA( APP_AUDIO_SAI,
									 &g_saiTxEdmaHandle,
									 &saiConfig);

		// set the transmitter bit clock rate configurations
		SAI_TxSetBitClockRate( APP_AUDIO_SAI,
							   APP_AUDIO_SAI_CLK_FREQ, 					 // SAI bit clock frequency requiered
							   (uint32_t) p_configParam->sampleRate,     // sample rate in Hz
							   (uint32_t) p_configParam->bitPerSample,   // bit width per sample
							   (uint32_t) p_configParam->channelNumber   // number of output channel
							  );

    } // end AUDIO_SINK_SPEAKER1 || AUDIO_SINK_SPEAKER2
	else
	{
		PRINTF("FAIL: audio Rx source not supported (initAudioRxPathHw / init path) \r\n");
		retStatus = kStatus_OutOfRange;
	}

    return retStatus;
}

#endif // end platform selection
