/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Board includes */
#include <stddef.h>

//Audio
#include "audio.h"
#include "audioTxRxTask.h"

// fsl general
#include "fsl_common.h"

//init
#include "init.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            initAudioSoftwareProcess										             */
/*                                                                                               */
/* DESCRIPTION: Init the software process of Tx and Rx path                                      */
/*               Search for process one by one then 											 */
/*                 1- check configuration is compatible with other process 						 */
/*                 2- create process instance													 */
/*                 3- set process parameters                                                     */
/*                                                                                               */
/* PARAMETERS:                                                                                   */
/*************************************************************************************************/
status_t initAudioSoftwareProcess(AUDIO_definition_st* p_audioDefinition)
{
	// error handle
	status_t 	 			 retStatus 	 	 = kStatus_Success;
	status_t 	 			 retStatusFunc   = kStatus_Success;

	// parameters
	AUDIO_definitionPath_st* p_audioTxPath_handle = &p_audioDefinition->audioTxPath_handle;
	AUDIO_definitionPath_st* p_audioRxPath_handle = &p_audioDefinition->audioRxPath_handle;

	volatile PL_INT16 iProcess = 0;

	/********************************************
     *  Check software process compatibility
     *    At this step software process parameters are checked with there associated path (Rx or Tx)
     *    It is time now to check if they are all compatible together
     */

#ifdef CONVERSA_PRESENT																		  // if conversa library is present
	/* Search for conversa process */
	iProcess = 0;
	while(iProcess < AUDIO_PROCESS_LIST_MAX)  										  // search for Conversa process inside Rx or Tx
	{
		if ( p_audioDefinition->processList[iProcess] == AUDIO_PROC_CONVERSA_TXRX )   // Conversa process is present either in Rx or Tx path
		{
			/*
			 * Set conversa parameters and create instance
			 */
			retStatusFunc = initSetConversa( &p_audioTxPath_handle->configParam,
											 &p_audioRxPath_handle->configParam,
											 &p_audioDefinition->swIpConversa_handle );
			if (retStatusFunc != kStatus_Success)
			{
				PRINTF("FAIL: Fail to init and create instance of conversa\r\n");
				retStatus = kStatus_Fail;
			}
			break; // stop looking for conversa process
		}
		iProcess = iProcess + 1;
	}
#endif

	return retStatus;
}
