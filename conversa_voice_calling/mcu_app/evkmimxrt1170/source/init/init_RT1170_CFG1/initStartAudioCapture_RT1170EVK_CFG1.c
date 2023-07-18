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

// Board
#include "board.h"

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

/*****************************************************************************************
 * Function
 *****************************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

/***********************************************************
 *
 *  Start INIT audio tx path source for RT1170EVK_REVA
 *
 ***********************************************************/
status_t INIT_startAudioCapture(AUDIO_definition_st* 	p_definition)
{
	AUDIO_workFlowHandle_st* p_audioWorkFlow_handle = &p_definition->audioWorkFlow_handle;
	AUDIO_definitionPath_st* p_audioTxPath_handle   = &p_definition->audioTxPath_handle;
	AUDIO_definitionPath_st* p_audioRxPath_handle	= &p_definition->audioRxPath_handle;


	status_t retStatus 		 = kStatus_Success;
	status_t retStatusFunc	 = kStatus_Success;

	// SAI
    sai_transfer_t saiXfer;

	/*
	 * Suspend tasks & scheduler
	 *   goal is to guarantee fix delay for synchronization
	 */
    vTaskSuspendAll();

	/*
	 * If start order is : SAI Tx then PDM start on a SAI Tx IT
	 */
	if (p_audioWorkFlow_handle->startOrder == AUDIO_START_ORDER_SAITX_PDM )
	{
		/*
		 * SYNCHRONOUS START TX / RX
		 *    - Pre load 2 Xfer queue to SAI tx
		 *    - Wait for source Tx received interrupt
		 *    - Start Sai Tx transfer
		 */
		/* Preload 2 transfer queue data to the sink */
		/* Add data to SAI Edma queue */
		saiXfer.data     =  (PL_INT8*) p_audioRxPath_handle->configParam.bufferHandle.p_pingBufferAddress[0];		// adress of the data
		saiXfer.dataSize = p_audioRxPath_handle->configParam.bytePerFrame; 											// byte per frame
		retStatusFunc = SAI_TransferSendEDMA( APP_AUDIO_SAI,
											  &g_saiTxEdmaHandle,			// sai tx edma handle
											  &saiXfer);					// sai transfer config
		if (retStatusFunc == kStatus_SAI_QueueFull)
		{
			retStatus = retStatusFunc;
			PRINTF("FAIL: start audio capture EDMA SAI queue full\r\n");
		}
		else if (retStatusFunc != kStatus_Success)
		{
			retStatus = retStatusFunc;
			PRINTF("FAIL: start audio capture EDMA SAI Error");
		}

		/* Add data to SAI Edma queue */
		saiXfer.data     =  (PL_INT8*) p_audioRxPath_handle->configParam.bufferHandle.p_pongBufferAddress[0];		// address of the data
		saiXfer.dataSize = p_audioRxPath_handle->configParam.bytePerFrame; 											// byte per frame
		retStatusFunc = SAI_TransferSendEDMA( APP_AUDIO_SAI,
											  &g_saiTxEdmaHandle,			// sai tx edma handle
											  &saiXfer);					// sai transfer config
		if (retStatusFunc == kStatus_SAI_QueueFull)
		{
			retStatus = retStatusFunc;
			PRINTF("FAIL: start audio capture EDMA SAI queue full\r\n");
		}
		else if (retStatusFunc != kStatus_Success)
		{
			retStatus = retStatusFunc;
			PRINTF("FAIL: start audio capture EDMA SAI Error");
		}

		/* Start PDM capture after a Rx path sink interrupt
		 *     pre-requis: Rx path sink (SAI Tx) is already running and generate interrupts
		 *                 g_startMechanismTxRx is reset on PONG interrupt
		 *                 Enable EDMA PDM receive flow service is located into SAI Tx interrupts to delay optimization
		 */
		g_startMechanismTxRx = 1;				// set to 1, it will be reset in the next PONG IT which is enabled
		while(g_startMechanismTxRx == 1);       // wait g_startMechanismTxRx is set to 0
		/* Enable EDMA PDM receive flow => Located into SAI Tx interrupt service routine to minimize the delay between Rx & Tx interrupt */

	} // end AUDIO_START_ORDER_SAITX_PDM
	else
	{
		retStatus = kStatus_OutOfRange;
	}

	/*
	 * Resume tasks & scheduler
	 */
	xTaskResumeAll();

	return retStatus;
}

#endif
