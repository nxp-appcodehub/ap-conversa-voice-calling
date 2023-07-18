/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Board includes */

//general
#include "PL_platformTypes.h"
#include "appGlobal.h"
#include "board.h" // for gpio

// IRQ & callback
#include "IRQ_callBack.h"

// debug
#include "fsl_debug_console.h"

// memory section replacement
#include <cr_section_macros.h>


/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*****************************************************************************************
 * Function
 *****************************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
__RAMFUNC(RAM) // force to put the code instruction of this function in RAM (FLEXRAMRAM_ITC)
void callBack_saiTx(I2S_Type *base, sai_edma_handle_t *handle, status_t status, void *userData)
{
	status_t 	 			 retStatusFunc   = kStatus_Success;

	// audio pointer
	AUDIO_definition_st*  	  p_definition  	= (AUDIO_definition_st*) userData;
	AUDIO_definitionPath_st*  p_definitionPath 	= &p_definition->audioRxPath_handle;
	AUDIO_workFlowHandle_st*  p_audioWorkFlow_handle   = &p_definition->audioWorkFlow_handle;
	// SAI
    sai_transfer_t saiXfer;
	// FreeRtos
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;


	/* According to Ping or Pong event
	 * 			- Add queue transfer element
	 * 			- Update global var
	 * 			- Update group event bit
	 */

    /* if ping transfer finished => current transfer is pong then
     *     Sw IT need to create a pong TCD in the queue system for next + 1 transfer
     *     SW audio task need to prepare ping buffer
     */

	if ( g_saiTxEdmaHandle.saiQueue[g_saiTxEdmaHandle.queueDriver].data == (PL_UINT8*) p_definitionPath->configParam.bufferHandle.p_pingBufferAddress[0] )
    {

		xEventGroupClearBitsFromISR( p_audioWorkFlow_handle->processEventGroup,									// clear AUDIO_TX_PDM_PONG_BUFF bit event
									 AUDIO_TX_SAI0_PONG_BUFF );

		xEventGroupSetBitsFromISR(	p_audioWorkFlow_handle->processEventGroup,										// set AUDIO_TX_SAI0_PING_BUFF bit event
									AUDIO_TX_SAI0_PING_BUFF,
									&xHigherPriorityTaskWoken);

		saiXfer.data     =  (PL_INT8*) p_definitionPath->configParam.bufferHandle.p_pongBufferAddress[0];	// Sw load a pong in EDMA TCD hardware system so add new pong transfer to the SW queue
	}

    /* if pong transfer finished => current transfer is ping then
     *     Sw IT need to create a ping TCD in the queue system for next + 1 transfer
     *     SW audio task need to prepare pong buffer
     */
    else
	{
		// Start the Tx source on top synchro
		if (g_startMechanismTxRx == 1)
		{
			if (g_appHandle.audioDefinition.audioWorkFlow_handle.startOrder == AUDIO_START_ORDER_SAITX_PDM) // If start sequence is EDMA SAI Tx 1st then EDMA PDM Rx
			{
				PDM_EnableDMA(APP_AUDIO_PDM_BASE_ADDRESS, true);											// Start PDM EDMA transfer

				g_startMechanismTxRx = 0;			 														// reset the Rx / Tx start mechanism when pong buffer is received
				// Enable PDM receive
				xEventGroupSetBitsFromISR(	p_audioWorkFlow_handle->processEventGroup,
											AUDIO_TX_PDM_PONG_BUFF,
											&xHigherPriorityTaskWoken); 									// set AUDIO_TX_PDM_PONG_BUFF bit event => first receive data will be PING data
			}
		}

    	xEventGroupClearBitsFromISR( p_audioWorkFlow_handle->processEventGroup,								// clear AUDIO_TX_PDM_PING_BUFF bit event
									 AUDIO_TX_SAI0_PING_BUFF );

		xEventGroupSetBitsFromISR(	p_audioWorkFlow_handle->processEventGroup,								// set AUDIO_TX_SAI0_PONG_BUFF bit event
									AUDIO_TX_SAI0_PONG_BUFF,
									&xHigherPriorityTaskWoken);

		saiXfer.data     =  (PL_INT8*) p_definitionPath->configParam.bufferHandle.p_pingBufferAddress[0];	// Sw load a ping in EDMA TCD hardware system so add new ping transfer to the SW queue
	}

	/*Add the transfert to EDMA queue */
	saiXfer.dataSize = p_definitionPath->configParam.bytePerFrame; 	// byte per frame
	retStatusFunc = SAI_TransferSendEDMA( APP_AUDIO_SAI,
										  &g_saiTxEdmaHandle,			// sai tx edma handle
										  &saiXfer);					// sai transfer config
	if (retStatusFunc == kStatus_SAI_QueueFull)
	{
		PRINTF("[AUDIO_TxRxTask] FAIL EDMA SAI queue full \r\n");
	}
	else if (retStatusFunc != kStatus_Success)
	{
		PRINTF("[AUDIO_TxRxTask] FAIL EDMA SAI Error");
	}

	xHigherPriorityTaskWoken = pdTRUE; 				// force the scheduler to run after the IT
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken ); // If pxHigherPriorityTaskWoken is pdTRUE, then scheduler is called.
}

