/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __initAudioUseCase_h___
#define __initAudioUseCase_h___

// general
#include "appGlobal.h"

// SSRC
#include "SSRC.h"

/*******************************************************************************
 * Macro Definitions
 ******************************************************************************/
// SSRC
extern LVM_INT32								mem[APP_AUDIO_TX_SAMPLE_PER_FRAME_RDSPMIC * 6];
extern SSRC_Scratch_t* 							pScratch;                   		// Pointer to SSRC scratch memory
extern SSRC_Instance_t 							SSRC_Instance;              		// Allocate memory for the instance
extern SSRC_Params_t   							SSRC_Params;                		// Memory for init parameters
extern LVM_INT32       							ScratchSize;                		// The size of the scratch memory
extern LVM_INT16*      							pInputInScratch;            		// Pointer to input in the scratch buffer
extern LVM_INT16*      							pOutputInScratch;           		// Pointer to the output in the scratch buffer
/*******************************************************************************
 *PROTOTYPES
 ******************************************************************************/
/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            initBoardClock										 					 */
/*                                                                                               */
/* DESCRIPTION: Init required board clock                                  		 				 */
/*                                                                                               */
/* PARAMETERS:                                                                                   */
/*************************************************************************************************/
status_t initBoardClock();
/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            initFlexcommPinsSharing										 			 */
/*                                                                                               */
/* DESCRIPTION: declare pin sharing                                 		 				     */
/*                                                                                               */
/* PARAMETERS:                                                                                   */
/*************************************************************************************************/
status_t initFlexcommPinsSharing(AUDIO_definition_st* 	 p_definition);
/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            initCodecWM8904										 			 		 */
/*                                                                                               */
/* DESCRIPTION: init Codec WM8904 + set volume                                		 		     */
/*                                                                                               */
/* PARAMETERS:                                                                                   */
/*************************************************************************************************/
#if RX_PATH_PRESENT
void initCodecWM8904(void);
#endif
/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            initAudioRxPathHw										 			 	 */
/*                                                                                               */
/* DESCRIPTION: Init audio Tx path clock & interface (I2S & EDMA                                 */
/*                                                                                               */
/* PARAMETERS:     																				 */
/*              p_definition                                                                     */
/*************************************************************************************************/
status_t initAudioTxPathHw( AUDIO_definition_st* p_definition );
/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            initAudioRxPathHw										 			 	 */
/*                                                                                               */
/* DESCRIPTION: Init audio Rx path clock & interface (I2S, USB) & EDMA                           */
/*                                                                                               */
/* PARAMETERS:     																				 */
/*              p_definition                                                                     */
/*************************************************************************************************/
#if RX_PATH_PRESENT
status_t initAudioRxPathHw( AUDIO_definition_st* p_definition );
#endif
/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            initAudioUseCase										 			 	 */
/*                                                                                               */
/* DESCRIPTION: TX/RX initialization function                           						 */
/*                                                                                               */
/* PARAMETERS:     																				 */
/*              p_audioDefinition_handle                                                         */
/*************************************************************************************************/
status_t initAudioUseCase(AUDIO_definition_st* p_audioDefinition_handle);
/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            initStartAudioCapture										 			 */
/*                                                                                               */
/* DESCRIPTION: start I2S RX/TX dma transfer for synchronization purpose                         */
/*                                                                                               */
/* PARAMETERS:     																				 */
/*              p_definition                                                                     */
/*************************************************************************************************/
status_t initStartAudioCapture(AUDIO_definition_st* 	 p_definition);


#endif


