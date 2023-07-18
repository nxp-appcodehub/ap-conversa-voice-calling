/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __INIT_H__
#define __INIT_H__

#include <stdbool.h>

// Codec
#include "fsl_codec_common.h"

// USB
#include "usb.h"
#include "usb_device_ch9.h"
#include "usb_phy.h"
#include "fsl_device_registers.h"

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

// Flex Ram
#include "fsl_flexram.h"
#include "fsl_flexram_allocate.h"

// Sw Ip: Conversa
#ifdef CONVERSA_PRESENT
  #include "RdspConversaPlugin.h"
  #include "RdspConversaPluginConfig.h"
#endif

/*******************************************************************************
 * Structure
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*****************************************************************************************
 * Function
 *****************************************************************************************/

/********************************** GENERAL APPLICATION **********************************/
/*
 * Init board and clock
 */
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
/* FUNCTION:            initBoarClockEnableSaiMclkOutput										 */
/*                                                                                               */
/* DESCRIPTION: enable disable Sai master clock output                                  		 */
/*                                                                                               */
/* PARAMETERS:                                                                                   */
/*************************************************************************************************/
status_t initBoarClockEnableSaiMclkOutput(bool enable);

/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            initAudioCodec															 */
/*                                                                                               */
/* DESCRIPTION: init the audio codec                                  		 					 */
/*                                                                                               */
/* PARAMETERS:                                                                                   */
/*************************************************************************************************/
status_t initAudioCodec( AUDIO_definitionPath_st* p_audioTxPath_handle,
						 AUDIO_definitionPath_st* p_audioRxPath_handle);

/*
 * Init audio Tx path clock & interface (SAI, PDM) & EDMA
 */
status_t initAudioTxPathHw( AUDIO_definition_st* p_definition );

/*
 * Init audio Rx path clock & interface (SAI, USB) & EDMA
 */
status_t initAudioRxPathHw( AUDIO_definition_st* p_definition );

/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            initFlexRamAllocation										             */
/*                                                                                               */
/* DESCRIPTION: reallocate flexram (DTCM,ITCM,OCRAM                                   			 */
/*               please make sure it is allign with project settings							 */
/* PARAMETERS:                                                                                   */
/*************************************************************************************************/
status_t initFlexRamAllocation(flexram_allocate_ram_t* p_ramAllocate);

/*********************************** CHECK PATH ********************************************/

/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            initCheckDefinitionPathTxRx										 		 */
/*                                                                                               */
/*************************************************************************************************/
status_t initCheckDefinitionPathTxRx(	AUDIO_definitionPath_st* p_definitionPathTx,
										AUDIO_definitionPath_st* p_definitionPathRx);

/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            INIT_startAudioCapture										             */
/*                                                                                               */
/* DESCRIPTION: start audio capture with respect of the required order					         */
/*              							 													 */
/* PARAMETERS:                                                                                   */
/*************************************************************************************************/
status_t INIT_startAudioCapture(AUDIO_definition_st* 	p_definition);

/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            initAudioSoftwareProcess										         */
/*                                                                                               */
/*************************************************************************************************/
status_t initAudioSoftwareProcess(AUDIO_definition_st* p_audioDefinition);

/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            initAudioUseCase										         		 */
/*                                                                                               */
/*************************************************************************************************/
status_t initAudioUseCase(AUDIO_definition_st* p_audioDefinition_handle);

/*********************************** CONVERSA ***************************************************/
#ifdef CONVERSA_PRESENT
/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            initCheckDefinitionParameterFileConversa								 */
/*                                                                                               */
/*************************************************************************************************/

status_t initCheckDefinitionParameterFileConversa( AUDIO_conversa_st* p_audioDefinition);

/**************************************************************************************************/
/*                                                                                                */
/* FUNCTION:            initCheckDefinitionPathTxRx 											  */
/*                                                                                                */
/**************************************************************************************************/

status_t initCheckDefinitionPathTxRxConversa( AUDIO_definitionPath_st* p_definitionPathTx,
											  AUDIO_definitionPath_st* p_definitionPathRx,
											  AUDIO_conversa_st*	   p_definitionConversa
										    );

/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            initSetConversa		   											 		 */
/*                                                                                               */
/*************************************************************************************************/
status_t initSetConversa( 	AUDIO_configParam_st*   p_configParamTx,
							AUDIO_configParam_st*   p_configParamRx,
							AUDIO_conversa_st* 		p_definitionConversa);

#endif

#endif /* __INIT_H__ */



