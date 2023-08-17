/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

//Standard headers
#include <init.h>
#include <stdint.h>

//Platform headers
#include "board.h"

//fsl driver headers
#include "fsl_debug_console.h"
#include "fsl_i2s_dma.h"

//Application headers
#include "appGlobal.h"
#include "IRQ_callBack.h"

extern volatile PL_UINT32 						  	g_startMechanismTxRx;
/*******************************************************************************
 * Code
 ******************************************************************************/
#if RX_PATH_PRESENT
__attribute__((__section__(".ramfunc*")))
void callBack_I2S_DMA_Rx(I2S_Type *base, i2s_dma_handle_t *handle, status_t completionStatus, void *userData)
{
	AUDIO_definition_st* 	            p_definition         = (AUDIO_definition_st*)userData;
	AUDIO_WF_PINGPONG_STATUS_en			I2SPingPongStatus    = p_definition->audioWorkFlow_handle.pingPongRxStatus;


    // Updating the Ping/Pong event group status
    if (I2SPingPongStatus == AUDIO_WF_PING)
    {
    	p_definition->audioWorkFlow_handle.pingPongRxStatus 							  = AUDIO_WF_PONG;


    }
    else
    {
    	p_definition->audioWorkFlow_handle.pingPongRxStatus 							  = AUDIO_WF_PING;



    }

}
#endif

__attribute__((__section__(".ramfunc*")))
void callBack_I2S_DMA_Tx(I2S_Type *base, i2s_dma_handle_t *handle, status_t completionStatus, void *userData)
{
	AUDIO_definition_st* 	            p_definition        = (AUDIO_definition_st*)userData;
	AUDIO_WF_PINGPONG_STATUS_en			I2SPingPongStatus   = p_definition->audioWorkFlow_handle.pingPongTxStatus;
    // Updating the Ping/Pong event group status
    if (I2SPingPongStatus == AUDIO_WF_PING)
    {
    	// Update I2S PING/PONG status
    	p_definition->audioWorkFlow_handle.pingPongTxStatus = AUDIO_WF_PONG;   // TCD is writing in PING, PONG is available

    }
    else
    {
    	// Update I2S PING/PONG status
    	p_definition->audioWorkFlow_handle.pingPongTxStatus = AUDIO_WF_PING;  // TCD is writing in PONG, PING is available

    }

}


