/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


//Standard headers

//Platform headers
#include <init.h>
#include "board.h"

//fsl driver headers
#include "fsl_debug_console.h"
#include "fsl_dma.h"
#include "fsl_i2s.h"
#include "fsl_i2s_dma.h"


//Application headers
#include "appGlobal.h"

#include "IRQ_callBack.h"

#if (APP_PLATFORM == APP_PL_LPC55S69EVK)
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
 status_t INIT_audioTxPathclock	    (AUDIO_definition_st* 	 p_definition);
 status_t INIT_audioTxPathSource    (AUDIO_definition_st* 	 p_definition);


/*******************************************************************************
 * Variables
 ******************************************************************************/

PL_UINT32           *p_I2SBuffTX;														 // Pointer to I2S Receiving channel 1  ( To receive audio from Mics)
i2s_transfer_t      s_I2STransferTX[APP_AUDIO_DMA_NUMBER_OF_LINKED_DESC];			 	 // I2S transfer descriptor
i2s_dma_handle_t    s_I2S_DMAHandleTX;													 // I2S_DMA instance
static dma_handle_t s_DMAHandleTx;													     // I2S_DMA instance
static i2s_config_t s_I2SConfigTx;															 // I2S config
DMA_ALLOCATE_LINK_DESCRIPTORS(s_I2S_DMADscrTx, APP_AUDIO_DMA_NUMBER_OF_LINKED_DESC); 	 // memory allocation for linked descriptor


/*******************************************************************************
 * Code
 ******************************************************************************/
status_t initAudioTxPathHw( AUDIO_definition_st* p_definition )
{
	AUDIO_definitionPath_st* p_definitionPath = &p_definition->audioTxPath_handle;

	status_t retStatus 		 = kStatus_Success;
	status_t retStatusFunc	 = kStatus_Success;
	if (p_definition == NULL)
	{
		return kStatus_NullPointer;
	}
    retStatusFunc = INIT_audioTxPathclock(p_definition);
	if (retStatusFunc != kStatus_Success)
	{
		PRINTF("FAIL: fail to init audio Tx clock for LPC55S69 \r\n");
		retStatus = kStatus_Fail;
	}

    retStatusFunc = INIT_audioTxPathSource(p_definition);
	if (retStatusFunc != kStatus_Success)
	{
		PRINTF("FAIL: fail to init audio Tx source for LPC55S69 \r\n");
		retStatus = kStatus_Fail;
	}

	return retStatus;
}

status_t INIT_audioTxPathclock	(AUDIO_definition_st* 	 p_definition)
{
	if (p_definition == NULL)
	{
		return kStatus_NullPointer;
	}
    /* I2S clocks */
    CLOCK_AttachClk(kMCLK_to_FLEXCOMM2);
    /* reset FLEXCOMM for I2S */
    RESET_PeripheralReset(kFC2_RST_SHIFT_RSTn);
    /* reset NVIC for FLEXCOMM1 to FLEXCOMM5 */
    NVIC_ClearPendingIRQ(FLEXCOMM2_IRQn);
	return kStatus_Success;
}
status_t INIT_audioTxPathSource (AUDIO_definition_st* 	 p_definition)
{

	/*
	 * SAI Tx + EDMA INIT
	 * 		- SAI tx is slave ans receive data from the microphone. It is using RX I2S functions ( Receiving I2S functions)
	 * 		- data are transfered by EDMA
	 *
	 * 	  => Audio  loop will create DMA transfer with ping pong buffer mechanism
	 */
	if (p_definition == NULL)
	{
		return kStatus_NullPointer;
	}
	// get default I2S config
    I2S_RxGetDefaultConfig(&s_I2SConfigTx);
    // update config to fit our usecase
    if (   ( g_appHandle.platformAndMaterials == APP_PLATFORM_MATERIALS_LPC55S69_TB2136_RDSPMICARRAY)
    	&& ( g_appHandle.audioDefinition.audioTxPath_handle.configParam.sampleRate == 16000)	)
    {
    	s_I2SConfigTx.divider     = APP_AUDIO_I2S_TX_CLOCK_DIVIDER;
    }
    else
    {
    	s_I2SConfigTx.divider     = APP_AUDIO_I2S_RX_CLOCK_DIVIDER;
    }
    s_I2SConfigTx.masterSlave = APP_AUDIO_I2S_TX_MODE;
    s_I2SConfigTx.dataLength  = APP_AUDIO_I2S_TX_BIT_PER_SAMPLE;		//was 16
    s_I2SConfigTx.frameLength = APP_AUDIO_I2S_TX_BIT_PER_FRAME;
    s_I2SConfigTx.position    =  0;
	// Init I2S
	I2S_RxInit(APP_AUDIO_I2S_TX_SOURCE1,
			   &s_I2SConfigTx);
	/*
	 * DMA INIT for I2S TX
	 *   - set I2S Tx dma channel
	 *   - set I2S Tx dma request
	 *   - create I2S tx handle
	 */
	// Enable required DMA channel
    DMA_EnableChannel(APP_AUDIO_DMA_BASE_ADDRESS,
    				  APP_AUDIO_I2S_TX_SOURCE1_CHANNEL);
    // Set priority for enabled dma channel
    DMA_SetChannelPriority(APP_AUDIO_DMA_BASE_ADDRESS,
    		               APP_AUDIO_I2S_TX_SOURCE1_CHANNEL,
						   kDMA_ChannelPriority3);
    // Create DMA Instance
    DMA_CreateHandle(&s_DMAHandleTx,
    		         APP_AUDIO_DMA_BASE_ADDRESS,
					 APP_AUDIO_I2S_TX_SOURCE1_CHANNEL);
    //Initialize transfer descriptor
    if (   ( g_appHandle.platformAndMaterials == APP_PLATFORM_MATERIALS_LPC55S69_TB2136_RDSPMICARRAY)
        	&& ( g_appHandle.audioDefinition.audioTxPath_handle.configParam.sampleRate == 16000)	)
    {
    	/*
		 * WORKAROUND: only for RetuneDSP Mic array boards at 16k
		 * 			   for 16k of LRCLK, the BCLK is 1.023 Mhz. it is not enough for these mics that require a PDM_CLK > 1 Mhz. Sometimes the signal can be lost.
		 * 			   Workaround : Mics are acquired at 32k, then Fixed point SSRC to 16k in audioTxRxProcess()
		 */
		s_I2STransferTX[0].data  	=  (uint8_t *)(p_definition->audioTxPath_handle.configParam.bufferHandle.p_pingBufferAddress[0]);
		s_I2STransferTX[0].dataSize =  (2*p_definition->audioTxPath_handle.configParam.samplePerFrame) *( p_definition->audioTxPath_handle.configParam.bitPerSample/8 )* APP_AUDIO_I2S_TX_MAX_MICS_PER_CH ;
		s_I2STransferTX[1].data  	=  (uint8_t *)(p_definition->audioTxPath_handle.configParam.bufferHandle.p_pongBufferAddress[0]);
		s_I2STransferTX[1].dataSize =  (2*p_definition->audioTxPath_handle.configParam.samplePerFrame) *( p_definition->audioTxPath_handle.configParam.bitPerSample/8 )* APP_AUDIO_I2S_TX_MAX_MICS_PER_CH ;
    }
    else
    {
    	s_I2STransferTX[0].data  	=  (uint8_t *)(p_definition->audioTxPath_handle.configParam.bufferHandle.p_pingBufferAddress[0]);
		s_I2STransferTX[0].dataSize =  p_definition->audioTxPath_handle.configParam.samplePerFrame *( p_definition->audioTxPath_handle.configParam.bitPerSample/8 )* APP_AUDIO_I2S_TX_MAX_MICS_PER_CH ;
		s_I2STransferTX[1].data  	=  (uint8_t *)(p_definition->audioTxPath_handle.configParam.bufferHandle.p_pongBufferAddress[0]);
		s_I2STransferTX[1].dataSize =  p_definition->audioTxPath_handle.configParam.samplePerFrame *( p_definition->audioTxPath_handle.configParam.bitPerSample/8 )* APP_AUDIO_I2S_TX_MAX_MICS_PER_CH ;
    }
    // Create I2S_DMA tranfer

	I2S_RxTransferCreateHandleDMA(APP_AUDIO_I2S_TX_SOURCE1,
								  &s_I2S_DMAHandleTX,
								  &s_DMAHandleTx,
								  callBack_I2S_DMA_Tx,
								  (void *)p_definition);
	// Install transfer descriptor loop in I2S_DMA instance
	I2S_TransferInstallLoopDMADescriptorMemory(&s_I2S_DMAHandleTX,
											  (void *)s_I2S_DMADscrTx,
											  APP_AUDIO_DMA_NUMBER_OF_LINKED_DESC);
	// Init buffers
	memset(g_sourceRawBuffPingTx,
		   0,
		   sizeof(g_sourceRawBuffPingTx));
	memset(g_sourceRawBuffPongTx,
		   0,
		   sizeof(g_sourceRawBuffPongTx));
	/*
	 * WORKAROUND: only for RetuneDSP Mic array boards at 16k
	 * 			   for 16k of LRCLK, the BCLK is 1.023 Mhz. it is not enough for these mics that require a PDM_CLK > 1 Mhz. Sometimes the signal can be lost.
	 * 			   Workaround : Mics are acquired at 32k, then Fixed point SSRC to 16k in audioTxRxProcess()
	 */
	if (   ( g_appHandle.platformAndMaterials == APP_PLATFORM_MATERIALS_LPC55S69_TB2136_RDSPMICARRAY)
		&& ( g_appHandle.audioDefinition.audioTxPath_handle.configParam.sampleRate == 16000)	)
	{
		SSRC_ReturnStatus_en  			 ReturnStatus;
		SSRC_Params.SSRC_Fs_In 			= LVM_FS_32000;
		SSRC_Params.SSRC_Fs_Out 		= LVM_FS_16000;
		SSRC_Params.SSRC_NrOfChannels	= LVM_STEREO;
		SSRC_Params.Quality 			= SSRC_QUALITY_VERY_HIGH;
		SSRC_Params.NrSamplesIn			= APP_AUDIO_TX_SAMPLE_PER_FRAME_RDSPMIC;
		SSRC_Params.NrSamplesOut		= g_appHandle.audioDefinition.audioTxPath_handle.configParam.samplePerFrame;
		ReturnStatus = SSRC_GetScratchSize(  &SSRC_Params,  &ScratchSize );
		if (ReturnStatus != SSRC_OK)
		{
			PRINTF("Error code %d returned by the SSRC_GetNrSamples function\r\n",(LVM_INT16)ReturnStatus);
			return kStatus_Fail;
		}

		pScratch = (SSRC_Scratch_t *) &SSRC_scratchMem[0];
		ReturnStatus = SSRC_Init(   &SSRC_Instance, pScratch, &SSRC_Params,
		                            &pInputInScratch, &pOutputInScratch);
		if (ReturnStatus != SSRC_OK)
		{
			PRINTF("Error code %d returned by the SSRC_Init function\n",(LVM_INT16)ReturnStatus);
			return kStatus_Fail;
		}
	}
	return kStatus_Success;
}

#endif /*#if (APP_PLATFORM == APP_PL_LPC55S69EVK)*/

