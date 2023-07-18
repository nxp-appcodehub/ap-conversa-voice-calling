/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SWPROCESS_H__
#define __SWPROCESS_H__

#include <stdbool.h>

// general
#include "appGlobal.h"

// debug
#include "fsl_debug_console.h"

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

/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            swProcessConversa			   										 	 */
/*                                                                                               */
/*************************************************************************************************/
#ifdef CONVERSA_PRESENT
	status_t swProcessConversa( rdsp_conversa_plugin_t*   p_conversaPluginParams,
								PL_INT8**   			  pp_inputAudioData_Tx,
								PL_INT8**   			  pp_inputAudioData_Rx,
								PL_INT8*   			  	  p_outputAudioData_Tx,
								PL_INT8**  			  	  pp_outputAudioData_Rx,
								CONVERSA_processFormat_en conversaProcess,
								PL_INT32				  inputAudioDataSize_sample
							   );
#endif

#endif /* __SWPROCESS_H__ */



