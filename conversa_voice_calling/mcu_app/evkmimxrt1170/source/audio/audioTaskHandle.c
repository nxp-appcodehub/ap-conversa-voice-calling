/*
 * Copyright 2020-2023 NXP
 * All rights reserved.
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

/*******************************************************************************
 * AUDIO_createTask
 ******************************************************************************/
status_t AUDIO_createTask(AUDIO_definition_st* p_audioHandle)
{
	status_t 	retStatus     	 	= kStatus_Success;
	BaseType_t 	returnStatus_task	= pdPASS;

	/************************************
	 *            CREATE TASK
	 ************************************/

	/* Create AUDIO Rx task if enabled */
	if (   (p_audioHandle->audioTxPath_handle.operatingMode == AUDIO_OM_ENABLE)
		|| (p_audioHandle->audioRxPath_handle.operatingMode == AUDIO_OM_ENABLE)
	   )
    {
		// Create new audio task
		PRINTF("****************************\r\n");
		PRINTF("Create new AUDIO TxRx task\r\n");
		PRINTF("****************************\r\n");
		returnStatus_task = xTaskCreate( AUDIO_TxRxTask,
										 "Audio Tx/Rx Task",
										 AUDIO_TASK_STACK_SIZE,
										 p_audioHandle,
										 AUDIO_TASK_PRIORITY,
										 &p_audioHandle->audioWorkFlow_handle.audioTask_handle);
		if (returnStatus_task != pdPASS)
		{
			PRINTF("Create AUDIO TxRx task FAIL\r\n");
			retStatus = kStatus_Fail;
		}
    }
	else
	{
		PRINTF("NO AUDIO Tx or Rx task required\r\n");
	}

	return retStatus;
}


/*******************************************************************************
 * AUDIO_deleteTask
 ******************************************************************************/
status_t AUDIO_deleteTask(AUDIO_definition_st* p_audioHandle)
{
	status_t 	retStatus     	 	= kStatus_Success;
	BaseType_t 	returnStatus_task	= pdPASS;

	/************************************
	 *            DELETE TASK
	 ************************************/

	/* Deletee AUDIO Rx task if enabled */
	if( p_audioHandle->audioWorkFlow_handle.audioTask_handle != NULL ) 				// Close the current audio task if exist

	{
		PRINTF("Deleting current AUDIO TxRx task: ...");
		vTaskDelete( p_audioHandle->audioWorkFlow_handle.audioTask_handle );

		PRINTF("Current AUDIO TxRx task deleted \r\n");
		retStatus = kStatus_Success;
	}
	else
	{
		PRINTF("No Audio Rx task to delete \r\n");
		retStatus = kStatus_Success;
	}

	return retStatus;
}

/*******************************************************************************
 * AUDIO_SetHandleDefault
 ******************************************************************************/
status_t AUDIO_SetHandleDefault(AUDIO_definition_st* p_audioHandle)
{
	PL_INT8 i = 0;

	/***************************/
	/* CHECK PARAMETER POINTER */
	if (p_audioHandle == NULL)
	{
		return kStatus_NullPointer;
	}

	memset(p_audioHandle,0,sizeof(AUDIO_definition_st));

	// RxPATH
	p_audioHandle->audioRxPath_handle.operatingMode				  		 = AUDIO_OM_DISABLE;
	p_audioHandle->audioRxPath_handle.configParam.source	      		 = AUDIO_SRC_NO;
	p_audioHandle->audioRxPath_handle.configParam.sink            		 = AUDIO_SINK_NO;
	p_audioHandle->audioRxPath_handle.configParam.sampleRate      		 = 48000;
	p_audioHandle->audioRxPath_handle.configParam.samplePerFrame  		 = 128;
	p_audioHandle->audioRxPath_handle.configParam.bitPerSample    		 = 16;
	p_audioHandle->audioRxPath_handle.configParam.channelNumber   		 = 1;
	p_audioHandle->audioRxPath_handle.configParam.bytePerFrame    		 = p_audioHandle->audioRxPath_handle.configParam.samplePerFrame * p_audioHandle->audioRxPath_handle.configParam.bitPerSample / 8 * p_audioHandle->audioRxPath_handle.configParam.channelNumber ;
	p_audioHandle->audioRxPath_handle.configParam.bytePerFramePerChannel = p_audioHandle->audioRxPath_handle.configParam.samplePerFrame * p_audioHandle->audioRxPath_handle.configParam.bitPerSample / 8;

	// TxPATH
	p_audioHandle->audioTxPath_handle.operatingMode				  		 = AUDIO_OM_DISABLE;
	p_audioHandle->audioTxPath_handle.configParam.source	      		 = AUDIO_SRC_NO;
	p_audioHandle->audioTxPath_handle.configParam.sink            		 = AUDIO_SINK_NO;
	p_audioHandle->audioTxPath_handle.configParam.sampleRate      		 = 48000;
	p_audioHandle->audioTxPath_handle.configParam.samplePerFrame  		 = 128;
	p_audioHandle->audioTxPath_handle.configParam.bitPerSample    		 = 16;
	p_audioHandle->audioTxPath_handle.configParam.channelNumber   		 = 1;
	p_audioHandle->audioTxPath_handle.configParam.bytePerFrame    		 = p_audioHandle->audioTxPath_handle.configParam.samplePerFrame * p_audioHandle->audioTxPath_handle.configParam.bitPerSample / 8 * p_audioHandle->audioTxPath_handle.configParam.channelNumber ;
	p_audioHandle->audioTxPath_handle.configParam.bytePerFramePerChannel = p_audioHandle->audioTxPath_handle.configParam.samplePerFrame * p_audioHandle->audioTxPath_handle.configParam.bitPerSample / 8;
	p_audioHandle->audioWorkFlow_handle.processEventGroup 				 = NULL;

#ifdef CONVERSA_PRESENT
	p_audioHandle->swIpConversa_handle.conversaTuningParamStruct.isThereTuningFile = PL_FALSE; // conversa parameter file not selected by default
#endif

	return kStatus_Success;
}
