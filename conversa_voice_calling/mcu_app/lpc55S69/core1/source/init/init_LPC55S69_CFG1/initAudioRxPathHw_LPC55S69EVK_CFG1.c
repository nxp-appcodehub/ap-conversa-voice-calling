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
#include "fsl_sysctl.h"

//Application headers
#include "appGlobal.h"

#include "IRQ_callBack.h"

#if  (APP_PLATFORM == APP_PL_LPC55S69EVK)

#if RX_PATH_PRESENT
/*******************************************************************************
 * Variables
 ******************************************************************************/
PL_UINT32 *                       p_I2SBuffRX;												    // Pointer to I2S Transmitting channel 0 ( to send audio to CODEC)
static dma_handle_t               s_DMAHandleRx;												// DMA instance
i2s_dma_handle_t                  s_I2S_DMAHandleRX;											// I2S_DMA instance
i2s_transfer_t                    s_I2STransferRX[APP_AUDIO_DMA_NUMBER_OF_LINKED_DESC];			// I2S transfer descriptor
static i2s_config_t               s_I2SConfigRx;												// I2S config
DMA_ALLOCATE_LINK_DESCRIPTORS(s_I2S_DMADscrRx,  APP_AUDIO_DMA_NUMBER_OF_LINKED_DESC);			// memory allocation for linked descriptor
/******************************************************************************
 * Function
 ******************************************************************************/
 status_t INIT_audioRxPathclock	    (AUDIO_definition_st* 	 p_definition);
 status_t INIT_audioRxPathSink	    (AUDIO_definition_st* 	 p_definition);


/*******************************************************************************
 * Code
 ******************************************************************************/
 /***********************************************************
  *
  *  INIT audio Rx path source for LPC55S69
  *
  ***********************************************************/
 status_t initAudioRxPathHw( AUDIO_definition_st* p_definition )
 {
	 // Error variables
	status_t retStatusFunc = kStatus_Success;
	status_t retStatus 	   = kStatus_Success;
	if (p_definition == NULL)
	{
		return kStatus_NullPointer;
	}
	retStatusFunc = INIT_audioRxPathclock( p_definition );
	if (retStatusFunc != kStatus_Success)
	{
		PRINTF("FAIL: fail to init audio Rx clock for LPC55S69\r\n");
		retStatus = kStatus_Fail;
	}

	retStatusFunc = INIT_audioRxPathSink( p_definition );
	if (retStatusFunc != kStatus_Success)
	{
		PRINTF("FAIL: fail to init audio Rx sink for LPC55S69 \r\n");
		retStatus = kStatus_Fail;
	}

 }
 /***********************************************************
   *
   *  INIT audio Rx path clock for LPC55S69
   *
   ***********************************************************/
status_t INIT_audioRxPathclock	(AUDIO_definition_st* 	 p_definition)
{
	if (p_definition == NULL)
	{
		return kStatus_NullPointer;
	}
    /* I2S clocks */
    CLOCK_AttachClk(kMCLK_to_FLEXCOMM7);
    /* reset FLEXCOMM for I2S */
    RESET_PeripheralReset(kFC7_RST_SHIFT_RSTn);
    /* reset NVIC for FLEXCOMM1 to FLEXCOMM5 */
    NVIC_ClearPendingIRQ(FLEXCOMM7_IRQn);
	return kStatus_Success;
}
/***********************************************************
  *
  *  INIT audio Rx path sink for LPC55S69
  *
  ***********************************************************/
status_t INIT_audioRxPathSink (AUDIO_definition_st* 	 p_definition)
{
	/*
	 * I2S Tx + EDMA INIT
	 * 		- I2S tx is master and send data to the the codec. So we use TX I2s functions ( transmission I2S functions)
	 * 		- data are transfered by EDMA
	 *
	 * 	  => Audio  main loop will create DMA transfer with ping pong buffer mechanism
	 */
	if (p_definition == NULL)
	{
		return kStatus_NullPointer;
	}
	// get default I2S config
    I2S_TxGetDefaultConfig(&s_I2SConfigRx);
    // update config to fit our usecase
    s_I2SConfigRx.divider     = APP_AUDIO_I2S_RX_CLOCK_DIVIDER;
    s_I2SConfigRx.masterSlave = APP_AUDIO_I2S_RX_MODE;
    s_I2SConfigRx.dataLength  = APP_AUDIO_I2S_RX_BIT_PER_SAMPLE;
    s_I2SConfigRx.frameLength = APP_AUDIO_I2S_RX_BIT_PER_FRAME;
    s_I2SConfigRx.position    =  0;
    // Init I2S
    I2S_TxInit(APP_AUDIO_I2S_RX_SINK,
    		   &s_I2SConfigRx);

    /*
	 * DMA INIT for I2S TX
	 *   - set I2S Tx dma channel
	 *   - set I2S Tx dma request
	 *   - create I2S tx handle
	 */
    // Enable required DMA channel
	DMA_EnableChannel(APP_AUDIO_DMA_BASE_ADDRESS,
					  APP_AUDIO_I2S_RX_SINK_CHANNEL);
	// Set priority for enabled dma channel
	DMA_SetChannelPriority(APP_AUDIO_DMA_BASE_ADDRESS,
						   APP_AUDIO_I2S_RX_SINK_CHANNEL,
						   kDMA_ChannelPriority1);
	// Create DMA Instance
	DMA_CreateHandle(&s_DMAHandleRx,
					 APP_AUDIO_DMA_BASE_ADDRESS,
					 APP_AUDIO_I2S_RX_SINK_CHANNEL);
	//Initialize transfer descriptor
	s_I2STransferRX[0].data  	= (uint8_t *)(p_definition->audioRxPath_handle.configParam.bufferHandle.p_pingBufferAddress[0]);
	s_I2STransferRX[0].dataSize = p_definition->audioRxPath_handle.configParam.samplePerFrame *p_definition->audioRxPath_handle.configParam.channelNumber *(p_definition->audioRxPath_handle.configParam.bitPerSample/8)  ;
	s_I2STransferRX[1].data  	= (uint8_t *)(p_definition->audioRxPath_handle.configParam.bufferHandle.p_pongBufferAddress[0]);
	s_I2STransferRX[1].dataSize = p_definition->audioRxPath_handle.configParam.samplePerFrame *p_definition->audioRxPath_handle.configParam.channelNumber *(p_definition->audioRxPath_handle.configParam.bitPerSample/8)  ;
	// Create I2S_DMA tranfer
	I2S_TxTransferCreateHandleDMA(APP_AUDIO_I2S_RX_SINK,
								  &s_I2S_DMAHandleRX,
								  &s_DMAHandleRx,
								  callBack_I2S_DMA_Rx,
								  (void *)p_definition);
	// Install transfer descriptor loop in I2S_DMA instance
	I2S_TransferInstallLoopDMADescriptorMemory(&s_I2S_DMAHandleRX,
											   (void *)s_I2S_DMADscrRx,
											   APP_AUDIO_DMA_NUMBER_OF_LINKED_DESC);
	/*
	 * Init Ping/Pong buffers
	 */
	memset(g_sinkBuffPingRx,
		   0,
		   sizeof(g_sinkBuffPingRx));
	memset(g_sinkBuffPingRx,
		   0,
		   sizeof(g_sinkBuffPongRx));
	return kStatus_Success;
}

#endif
#endif/*#if (APP_PLATFORM == APP_PL_LPC55S69EVK)*/
