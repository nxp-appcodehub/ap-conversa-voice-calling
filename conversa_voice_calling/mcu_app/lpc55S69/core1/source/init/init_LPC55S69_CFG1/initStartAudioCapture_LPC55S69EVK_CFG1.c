/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// general
#include "appGlobal.h"


// hardrware
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

// application
#include "init.h"

// tools
#include "tools.h"

// Board
#include "board.h"

// I2S & DMA
#include "fsl_dma.h"
#include "fsl_i2s.h"
#include "fsl_i2s_dma.h"

#if (APP_PLATFORM == APP_PL_LPC55S69EVK)

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
extern i2s_transfer_t     s_I2STransferTX[2];
#if RX_PATH_PRESENT
extern i2s_transfer_t     s_I2STransferRX[2];
#endif
extern i2s_dma_handle_t   s_I2S_DMAHandleTX;
extern i2s_dma_handle_t   s_I2S_DMAHandleRX;

/*****************************************************************************************
 * Function
 *****************************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            initStartAudioCapture										 			 */
/*                                                                                               */
/* DESCRIPTION: start I2S RX/TX dma transfer for synchronization purpose                         */
/*                                                                                               */
/* PARAMETERS:     																				 */
/*              p_definition                                                                     */
/*************************************************************************************************/
status_t initStartAudioCapture(AUDIO_definition_st* 	 p_definition)
{
	status_t retStatus = kStatus_Success;
	//fill the tx fifo to full (write 8 PL_INT32 ZERO samples), this is to let all the tx and rx interrupts synchronized (occur at the "same" time)
	//without these, the main audio processing will have to finish generating all the output samples earlier!
	//this is to let the main audio processing occupy the most time of 1 audio frame
	//the processing can finish at the latest closing point of one frame
	if (p_definition == NULL)
	{
		return kStatus_NullPointer;
	}
#if RX_PATH_PRESENT
	APP_AUDIO_I2S_RX_SINK->FIFOWR = 0U;
	APP_AUDIO_I2S_RX_SINK->FIFOWR = 0U;
	APP_AUDIO_I2S_RX_SINK->FIFOWR = 0U;
	APP_AUDIO_I2S_RX_SINK->FIFOWR = 0U;
	APP_AUDIO_I2S_RX_SINK->FIFOWR = 0U;
	APP_AUDIO_I2S_RX_SINK->FIFOWR = 0U;
	APP_AUDIO_I2S_RX_SINK->FIFOWR = 0U;
	APP_AUDIO_I2S_RX_SINK->FIFOWR = 0U;
#endif
	// Prepare TX RX transfer.
	I2S_TransferReceiveLoopDMA(APP_AUDIO_I2S_TX_SOURCE1,
							   &s_I2S_DMAHandleTX,
							   s_I2STransferTX,
							   APP_AUDIO_DMA_NUMBER_OF_LINKED_DESC);
#if RX_PATH_PRESENT
	I2S_TransferSendLoopDMA(APP_AUDIO_I2S_RX_SINK,
							&s_I2S_DMAHandleRX,
							s_I2STransferRX,
							APP_AUDIO_DMA_NUMBER_OF_LINKED_DESC);		//tx is configured to be the I2S clk master source, it should be enabled at last
#endif
	//g_startMechanismTxRx = 1;
	__disable_irq();
	DMA_StartTransfer(s_I2S_DMAHandleTX.dmaHandle);
#if RX_PATH_PRESENT
	DMA_StartTransfer(s_I2S_DMAHandleRX.dmaHandle);
#endif
	__enable_irq();

	return retStatus;
}

#endif
