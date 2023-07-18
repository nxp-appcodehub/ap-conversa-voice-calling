/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// general
#include "appGlobal.h"

//IRQ and callback
#include "IRQ_callBack.h"

// hardrware
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

// debug
#include "fsl_debug_console.h"

// DMA
#include "fsl_dmamux.h"

// PDM
#include "fsl_pdm.h"
#include "fsl_pdm_edma.h"

// application
#include "init.h"

// tools
#include "tools.h"

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

AT_NONCACHEABLE_SECTION_ALIGN(pdm_edma_handle_t 	s_pdmTxHandle      , 4);
AT_NONCACHEABLE_SECTION_ALIGN(edma_handle_t 		s_edmaHandle_PDM   , 4);
AT_NONCACHEABLE_SECTION_ALIGN(edma_tcd_t 			s_edmaTcd[2]  	   , 32U);
AT_NONCACHEABLE_SECTION_ALIGN(pdm_edma_transfer_t 	s_pdmEdmaTCD[1][2] , 32); 		// TCD definition transfert [microphone][pingPong]

/*****************************************************************************************
 * Function
 *****************************************************************************************/
status_t INIT_audioTxPathclock	(AUDIO_definition_st* 	 p_definition);
status_t INIT_audioTxPathSource (AUDIO_definition_st* 	 p_definition);
status_t INIT_audioTxPathSink	(AUDIO_definition_st* 	 p_definition);

/*******************************************************************************
 * Code
 ******************************************************************************/
/*
 * initAudioTxPathHw: init hardware Tx path (SAI, EDMA, PDM, ...)
 */
status_t initAudioTxPathHw( AUDIO_definition_st* p_definition )
{
	AUDIO_definitionPath_st* p_definitionPath = &p_definition->audioTxPath_handle;

	status_t retStatus 		 = kStatus_Success;
	status_t retStatusFunc	 = kStatus_Success;

    retStatusFunc = INIT_audioTxPathclock(p_definition);
	if (retStatusFunc != kStatus_Success)
	{
		PRINTF("FAIL: fail to init audio Tx clock for RT1170EVK_REVA \r\n");
		retStatus = kStatus_Fail;
	}

    retStatusFunc = INIT_audioTxPathSource(p_definition);
	if (retStatusFunc != kStatus_Success)
	{
		PRINTF("FAIL: fail to init audio Tx source for RT1170EVK_REVA \r\n");
		retStatus = kStatus_Fail;
	}

    retStatusFunc =	INIT_audioTxPathSink(p_definition);
	if (retStatusFunc != kStatus_Success)
	{
		PRINTF("FAIL: fail to init audio Tx sink for RT1170EVK_REVA \r\n");
		retStatus = kStatus_Fail;
	}
	return retStatus;
}

/***********************************************************
 *
 *  INIT audio tx path source for RT1170EVK_REVA
 *
 ***********************************************************/
status_t INIT_audioTxPathclock( AUDIO_definition_st*  p_definition )
{
	AUDIO_configParam_st* 	 p_configParam    			 = &p_definition->audioTxPath_handle.configParam;
	status_t retStatus 		 = kStatus_Success;

	/************************************************************************************
	 * Clock init
	 */
	/*if (p_configParam->source == AUDIO_SRC_AMIC1)
	{
		// Clock setting for SAI1
		CLOCK_SetRootClockMux(kCLOCK_Root_Sai1, 4);
		CLOCK_SetRootClockDiv(kCLOCK_Root_Sai1, 16);
		// Enable MCLK clock
		initBoarClockEnableSaiMclkOutput(true);
	}*/
	if 		(   (p_configParam->source == AUDIO_SRC_OMNI_1DMIC				)			// if PDM mic are used
			 || (p_configParam->source == AUDIO_SRC_LINEAR_2DMIC			)
			 || (p_configParam->source == AUDIO_SRC_TRIANGULAR_3DMIC		)
			 || (p_configParam->source == AUDIO_SRC_SQUARE_TRILLIUM_4DMIC	)
			)
	{
		/*
		 * Clock root setting for PDM mic
		 * 			Select audio PLL clock as input (reminder: AUDIO PLL clock = 393.24 MHz)
		 * 			Configure the PDM mic root clock to 24.576 MHz
		 */
		 CLOCK_SetRootClockMux(kCLOCK_Root_Mic, 6);		// PDM clock input module is Audio Pll clock => 393.24 MHz
		 CLOCK_SetRootClockDiv(kCLOCK_Root_Mic, 16);    // PDM clock input is divide by 16 => 393.24 MHz / 16 = 24.576 MHz
	}
	else
	{
		PRINTF("FAIL: audio Tx source not supported (initAudioTxPathHw / clock init) \r\n");
		retStatus = kStatus_OutOfRange;
	}

    /* end clock init */

	return retStatus;
}

/***********************************************************
 *
 *  INIT audio tx path source for RT1170EVK_REVA
 *
 ***********************************************************/
status_t INIT_audioTxPathSource(AUDIO_definition_st* 	p_definition)
{
	AUDIO_definitionPath_st* p_definitionPath 	   = &p_definition->audioTxPath_handle;
	AUDIO_configParam_st* 	 p_configParam    	   = &p_definitionPath->configParam;
	AUDIO_workFlowHandle_st* p_audioWorkFlow_handle= &p_definition->audioWorkFlow_handle;

	status_t retStatus 		 = kStatus_Success;
	status_t retStatusFunc	 = kStatus_Success;


	/*
	 * platform and materials dependency
	 */
	if ( g_appHandle.platformAndMaterials == 			APP_PLATFORM_MATERIALS_RT1170 )
	{
		g_pdmChannelConfig.gain = kPDM_DfOutputGain7; 			// gain 30dB with high quality
	}
	else if ( g_appHandle.platformAndMaterials == 		APP_PLATFORM_MATERIALS_RT1170_TB2136 )
	{
		g_pdmChannelConfig.gain = kPDM_DfOutputGain2;    		// gain 0dB with high quality
	}
	else if ( g_appHandle.platformAndMaterials == 		APP_PLATFORM_MATERIALS_RT1170_SHB_AMP2 )
	{
		g_pdmChannelConfig.gain = kPDM_DfOutputGain2;    		// gain 0dB with high quality
	}
	else
	{
		PRINTF("FAIL: PDM configure platform not defined.\r\n");
		return kStatus_Fail;
	}

	/*
	 * Source init
	 */
	if (    (p_configParam->source == AUDIO_SRC_OMNI_1DMIC				)			// if PDM mic are used
		 || (p_configParam->source == AUDIO_SRC_LINEAR_2DMIC			)
		 || (p_configParam->source == AUDIO_SRC_TRIANGULAR_3DMIC		)
		 || (p_configParam->source == AUDIO_SRC_SQUARE_TRILLIUM_4DMIC	)
		)
	{

	    // DMA MUX init already done
	    // DMA CONFIG already done

		/*
		 * Setup DMAMUX
		 *   DMA channel DEMO_EDMA_CHANNEL_PDM_MIC set to PDM mic1
		 */
		DMAMUX_SetSource	(	APP_AUDIO_DMA_DMAMUX,						// Set EDMA channel reserved to PDM to request source pdm mic
								APP_AUDIO_EDMA_CHANNEL_PDM_MIC,
								APP_AUDIO_EDMA_REQUEST_SOURCE_PDM_MIC);
		DMAMUX_EnableChannel( 	APP_AUDIO_DMA_DMAMUX,						// enable EDMA channel reserved to PDM
								APP_AUDIO_EDMA_CHANNEL_PDM_MIC);

		/*
		 * Set DMA IRQ priority
		 */
		NVIC_SetPriority(DMA2_DMA18_IRQn,
						 IRQ_PRIORITY_TX_PDM_DMA);

		/* Setup pdm & edma
		 *    PDM automatically send data to edma (minor edma loop)
		 *    edma will trigger IT when major loop done. IRQ is catched internaly inside the fsl driver then driver call the the callback when frame is received
		 *         - the minor transfer is correponding to PDM fifo size
		 *         - the major transfert is 1 audio frame receive
		 */
		EDMA_CreateHandle(		&s_edmaHandle_PDM,					// create EDMA handle for channel DEMO_EDMA_CHANNEL_PDM_MIC reserved for PDM mic
								APP_AUDIO_DMA_BASE_ADDRESS,
								APP_AUDIO_EDMA_CHANNEL_PDM_MIC);

		PDM_Init(	APP_AUDIO_PDM_BASE_ADDRESS,
					&g_pdmConfig);    								// init PDM with g_pdmConfig configuration

		PDM_TransferCreateHandleEDMA(APP_AUDIO_PDM_BASE_ADDRESS,
									 &s_pdmTxHandle,				// create PDM handle
									 callBack_edmaPdmMics,			// Callback of the Edma major loop associated to PDM mic => function called when a major transfer is done
									 p_audioWorkFlow_handle,  		// userdata parameter send in callback function
									 &s_edmaHandle_PDM);			// give the edma handle reserved for PDM

		PDM_TransferInstallEDMATCDMemory(&s_pdmTxHandle,			// set TCD memory for PDM
										 s_edmaTcd,
										 2);						// 2 TCD linked to manage ping pong buffer

		/* Configure TCD for PDM MIC 1 */
		s_pdmEdmaTCD[0][0].data 	    = (uint8_t *)(p_definitionPath->configParam.bufferHandle.p_pingBufferAddress[0]);      // next transfer will be stored into ping buffer
		// it should be this line s_pdmEdmaTCD[0][0].dataSize     = p_definitionPath->configParam.samplePerFrame * p_definitionPath->configParam.channelNumber * APP_AUDIO_PDM_BIT_PER_SAMPLE / 8 ;     // Number of byte per frame at PDM capture level. APP_AUDIO_PDM_BIT_PER_SAMPLE could be different than configParam.bitPerSample
		// but channelNumber is different to APP_AUDIO_PDM_CHANNEL_NUMBER_MAX and EDMA always read the APP_AUDIO_PDM_CHANNEL_NUMBER_MAX of PDM channel even if channel is off
		// So we always capture APP_AUDIO_PDM_CHANNEL_NUMBER_MAX channel mics and pick up the signal later
		s_pdmEdmaTCD[0][0].dataSize     = p_definitionPath->configParam.samplePerFrame * APP_AUDIO_PDM_CHANNEL_NUMBER_MAX * APP_AUDIO_PDM_BIT_PER_SAMPLE / 8;     // Number of byte per frame at PDM capture level. APP_AUDIO_PDM_BIT_PER_SAMPLE could be different than configParam.bitPerSample
		s_pdmEdmaTCD[0][0].linkTransfer =  &s_pdmEdmaTCD[0][1];

		// next TCD to execute is the pong MIC1
		s_pdmEdmaTCD[0][1].data 	    = (uint8_t *)(p_definitionPath->configParam.bufferHandle.p_pongBufferAddress[0]);      // next transfer will be stored into ping buffer
		// it should be this line s_pdmEdmaTCD[0][1].dataSize     = p_definitionPath->configParam.samplePerFrame * p_definitionPath->configParam.channelNumber * APP_AUDIO_PDM_BIT_PER_SAMPLE / 8 ;     // Number of byte per frame at PDM capture level. APP_AUDIO_PDM_BIT_PER_SAMPLE could be different than configParam.bitPerSample
		// but channelNumber is different to APP_AUDIO_PDM_CHANNEL_NUMBER_MAX and EDMA always read the APP_AUDIO_PDM_CHANNEL_NUMBER_MAX of PDM channel even if channel is off
		// So we always capture APP_AUDIO_PDM_CHANNEL_NUMBER_MAX channel mics and pick up the signal later
		s_pdmEdmaTCD[0][1].dataSize     = p_definitionPath->configParam.samplePerFrame * APP_AUDIO_PDM_CHANNEL_NUMBER_MAX * APP_AUDIO_PDM_BIT_PER_SAMPLE / 8;     // Number of byte per frame at PDM capture level. APP_AUDIO_PDM_BIT_PER_SAMPLE could be different than configParam.bitPerSample
		s_pdmEdmaTCD[0][1].linkTransfer = &s_pdmEdmaTCD[0][0];

		// configure the PDM + EDMA channel according to the number of channel used
		switch(p_definitionPath->configParam.channelNumber)
		{
		case 1:
			PDM_TransferSetChannelConfigEDMA(APP_AUDIO_PDM_BASE_ADDRESS,	// Configures the PDM channel(DEMO_PDM_ENABLE_CHANNEL_0) with g_pdmChannelConfig config
											 &s_pdmTxHandle,
											 APP_AUDIO_PDM_ENABLE_CHANNEL_0,
											 &g_pdmChannelConfig);
		break;
		case 2:
			PDM_TransferSetChannelConfigEDMA(APP_AUDIO_PDM_BASE_ADDRESS,	// Configures the PDM channel(DEMO_PDM_ENABLE_CHANNEL_0) with g_pdmChannelConfig config
											 &s_pdmTxHandle,
											 APP_AUDIO_PDM_ENABLE_CHANNEL_0,
											 &g_pdmChannelConfig);

			PDM_TransferSetChannelConfigEDMA(APP_AUDIO_PDM_BASE_ADDRESS,	// Configures the PDM channel(DEMO_PDM_ENABLE_CHANNEL_1) with g_pdmChannelConfig config
											 &s_pdmTxHandle,
											 APP_AUDIO_PDM_ENABLE_CHANNEL_1,
											 &g_pdmChannelConfig);
		break;
		case 3:
			PDM_TransferSetChannelConfigEDMA(APP_AUDIO_PDM_BASE_ADDRESS,	// Configures the PDM channel(DEMO_PDM_ENABLE_CHANNEL_0) with g_pdmChannelConfig config
											 &s_pdmTxHandle,
											 APP_AUDIO_PDM_ENABLE_CHANNEL_0,
											 &g_pdmChannelConfig);

			PDM_TransferSetChannelConfigEDMA(APP_AUDIO_PDM_BASE_ADDRESS,	// Configures the PDM channel(DEMO_PDM_ENABLE_CHANNEL_1) with g_pdmChannelConfig config
											 &s_pdmTxHandle,
											 APP_AUDIO_PDM_ENABLE_CHANNEL_1,
											 &g_pdmChannelConfig);
			PDM_TransferSetChannelConfigEDMA(APP_AUDIO_PDM_BASE_ADDRESS,	// Configures the PDM channel(DEMO_PDM_ENABLE_CHANNEL_4) with g_pdmChannelConfig config
											 &s_pdmTxHandle,
											 APP_AUDIO_PDM_ENABLE_CHANNEL_4,
											 &g_pdmChannelConfig);
		break;
		case 4:
			PDM_TransferSetChannelConfigEDMA(APP_AUDIO_PDM_BASE_ADDRESS,	// Configures the PDM channel(DEMO_PDM_ENABLE_CHANNEL_0) with g_pdmChannelConfig config
											 &s_pdmTxHandle,
											 APP_AUDIO_PDM_ENABLE_CHANNEL_0,
											 &g_pdmChannelConfig);
			PDM_TransferSetChannelConfigEDMA(APP_AUDIO_PDM_BASE_ADDRESS,	// Configures the PDM channel(DEMO_PDM_ENABLE_CHANNEL_1) with g_pdmChannelConfig config
											 &s_pdmTxHandle,
											 APP_AUDIO_PDM_ENABLE_CHANNEL_1,
											 &g_pdmChannelConfig);
			PDM_TransferSetChannelConfigEDMA(APP_AUDIO_PDM_BASE_ADDRESS,	// Configures the PDM channel(DEMO_PDM_ENABLE_CHANNEL_4) with g_pdmChannelConfig config
											 &s_pdmTxHandle,
											 APP_AUDIO_PDM_ENABLE_CHANNEL_4,
											 &g_pdmChannelConfig);
			PDM_TransferSetChannelConfigEDMA(APP_AUDIO_PDM_BASE_ADDRESS,	// Configures the PDM channel(DEMO_PDM_ENABLE_CHANNEL_5) with g_pdmChannelConfig config
											 &s_pdmTxHandle,
											 APP_AUDIO_PDM_ENABLE_CHANNEL_5,
											 &g_pdmChannelConfig);
		break;
		} // end of switch


		// set sample rate
		retStatusFunc = PDM_SetSampleRateConfig(	APP_AUDIO_PDM_BASE_ADDRESS,	   // set the PDM sample rate
													APP_AUDIO_PDM_CLK_FREQ,        // PDM clock frequency
													p_configParam->sampleRate);    // PDM Sample rate
		if (retStatusFunc != kStatus_Success)
		{
			PRINTF("FAIL: PDM configure sample rate failed.\r\n");
			return kStatus_Fail;
		}

		// Reset the PDM IP to force new configuration
		PDM_Reset(APP_AUDIO_PDM_BASE_ADDRESS);
		// PDM_Enable(DEMO_PDM, true);	// must be synchro with other audio flow

		/*
		// enable PDM IRQ for debug only or error management as EDMA is used
		PDM_EnableInterrupts(APP_AUDIO_PDM_BASE_ADDRESS, kPDM_ErrorInterruptEnable | kPDM_FIFOInterruptEnable); // for debug only as the EDMA is used
		EnableIRQ(PDM_EVENT_IRQn); 		// enable global IRQ PDM (error)
		#if !(defined FSL_FEATURE_PDM_HAS_NO_INDEPENDENT_ERROR_IRQ && FSL_FEATURE_PDM_HAS_NO_INDEPENDENT_ERROR_IRQ)
		EnableIRQ(PDM_ERROR_IRQn); 			// enable global IRQ PDM (event)
		#endif
		*/

		// start PDM + EDMA transfer with the first TCD
		retStatusFunc = PDM_TransferReceiveEDMA(APP_AUDIO_PDM_BASE_ADDRESS,
												&s_pdmTxHandle,
												&s_pdmEdmaTCD[0][0]);
		if (retStatusFunc != kStatus_Success)
		{
			PRINTF("FAIL: PDM EDMA transfer with 1st TCD \r\n");
			return kStatus_Fail;
		}

	} // end
	else
	{
		PRINTF("FAIL: audio Tx source not supported (initAudioTxPathHw / init path) \r\n");
		retStatus = kStatus_OutOfRange;
	}

	 return retStatus;
}

/***********************************************************
 *
 *  INIT audio tx path sink for RT1170EVK_REVA
 *
 ***********************************************************/
status_t INIT_audioTxPathSink(AUDIO_definition_st* 	 p_definition)
{
	AUDIO_configParam_st* 	 p_configParam    			 = &p_definition->audioTxPath_handle.configParam;
	status_t retStatus 		 = kStatus_Success;

	return kStatus_Success;
}

#endif
