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

// IRQ & callback
#include "IRQ_callBack.h"

// debug
#include "board.h" // debug (gpio)
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
static volatile bool s_lowFreqFlag          = false;
static volatile bool s_fifoErrorFlag        = false;

/*******************************************************************************
 * Code
 ******************************************************************************/
/*
 * callBack_pdmEdma
 */
__RAMFUNC(RAM) // force to put the code instruction of this function in RAM (FLEXRAMRAM_ITC)
void callBack_edmaPdmMics(PDM_Type *base, pdm_edma_handle_t *handle, status_t status, void *userData)
{
	// audio pointer
	volatile AUDIO_workFlowHandle_st*  p_audioWorkFlow_handle  = (volatile AUDIO_workFlowHandle_st*) userData;

	// FreeRtos
	BaseType_t   xHigherPriorityTaskWoken = pdFALSE;
	EventBits_t  eventBitsCurrent;

	eventBitsCurrent = xEventGroupGetBitsFromISR(p_audioWorkFlow_handle->processEventGroup);
	eventBitsCurrent = eventBitsCurrent & AUDIO_TX_PDM_PING_BUFF;

	if (eventBitsCurrent == AUDIO_TX_PDM_PING_BUFF)									// last event bit receive was PING
	{
		xEventGroupClearBitsFromISR( p_audioWorkFlow_handle->processEventGroup,		// clear AUDIO_TX_PDM_PING_BUFF bit event
									 AUDIO_TX_PDM_PING_BUFF );

		xEventGroupSetBitsFromISR(	p_audioWorkFlow_handle->processEventGroup,		// set AUDIO_TX_PDM_PONG_BUFF bit event
									AUDIO_TX_PDM_PONG_BUFF,
									&xHigherPriorityTaskWoken);
	}
	else																			// last event bit receive was PONG
	{

		xEventGroupClearBitsFromISR( p_audioWorkFlow_handle->processEventGroup,		// clear AUDIO_TX_PDM_PONG_BUFF bit event
									 AUDIO_TX_PDM_PONG_BUFF );

		xEventGroupSetBitsFromISR(	p_audioWorkFlow_handle->processEventGroup,		// set AUDIO_TX_PDM_PING_BUFF bit event
									AUDIO_TX_PDM_PING_BUFF,
									&xHigherPriorityTaskWoken);
	}

	xHigherPriorityTaskWoken = pdTRUE;			    // Force scheduler to run after the IT
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken ); // If pxHigherPriorityTaskWoken is pdTRUE, then a higher priority task need to be unlocked.
}

/* Function not use any more
 *    Still present for debug purpose
 */
#if !(defined FSL_FEATURE_PDM_HAS_NO_INDEPENDENT_ERROR_IRQ && FSL_FEATURE_PDM_HAS_NO_INDEPENDENT_ERROR_IRQ)
void PDM_ERROR_IRQHandler(void)
{
    uint32_t status = 0U;
    if (PDM_GetStatus(APP_AUDIO_PDM_BASE_ADDRESS) & PDM_STAT_LOWFREQF_MASK)
    {
        PDM_ClearStatus(APP_AUDIO_PDM_BASE_ADDRESS, PDM_STAT_LOWFREQF_MASK);
        s_lowFreqFlag = true;
        // PRINTF("y\r\n");
    }

    status = PDM_GetFifoStatus(APP_AUDIO_PDM_BASE_ADDRESS);
    if (status != 0U)
    {
        PDM_ClearFIFOStatus(APP_AUDIO_PDM_BASE_ADDRESS, status);
        s_fifoErrorFlag = true;
        // PRINTF("x\r\n");
    }
#if defined(FSL_FEATURE_PDM_HAS_RANGE_CTRL) && FSL_FEATURE_PDM_HAS_RANGE_CTRL
    status = PDM_GetRangeStatus(APP_AUDIO_PDM_BASE_ADDRESS);
    if (status != 0U)
    {
        PDM_ClearRangeStatus(APP_AUDIO_PDM_BASE_ADDRESS, status);
        // PRINTF("z\r\n");
    }
#else
    status = PDM_GetOutputStatus(APP_AUDIO_PDM_BASE_ADDRESS);
    if (status != 0U)
    {
        PDM_ClearOutputStatus(APP_AUDIO_PDM_BASE_ADDRESS, status);
    }
#endif
    __DSB();
}
#endif

/* Function not use any more
 *    Still present for debug purpose
 */
void PDM_EVENT_IRQHandler(void)
{
    uint32_t i = 0U, status = PDM_GetStatus(APP_AUDIO_PDM_BASE_ADDRESS);
    PDM_ClearStatus(APP_AUDIO_PDM_BASE_ADDRESS, status);
    __DSB();
}

