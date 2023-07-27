/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include <stdint.h>

// Debug
#include "fsl_debug_console.h"

// DMA
#include "fsl_dmamux.h"

// Shell
#include "fsl_shell.h"
#include "commandShell.h"

// application
#include "appGlobal.h"

// init
#include "init.h"

// USB
#include "usb_init.h"

// audio
#include "audioTxRxTask.h"

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
status_t initAudioUseCase(AUDIO_definition_st* p_audioDefinition_handle)
{
	// error handle
	status_t retStatus 	   = kStatus_Success;
	status_t retStatusFunc = kStatus_Success;

	AUDIO_definitionPath_st* p_audioRxPath_handle 	   = &p_audioDefinition_handle->audioRxPath_handle;
	AUDIO_definitionPath_st* p_audioTxPath_handle 	   = &p_audioDefinition_handle->audioTxPath_handle;
	AUDIO_workFlowHandle_st* p_audioWorkFlow_handle    = &p_audioDefinition_handle->audioWorkFlow_handle;

	// edma
	edma_config_t 	dmaConfig = {0};
	// circular buffers
	TOOLS_circularBuffer_param cbParamRX;
	TOOLS_circularBuffer_param cbParamTX;

	/*******************************************
	 * Init hardware general part
	 */
	// For RT1170 
	// DMAMUX & EDMA ( init + default config )
	DMAMUX_Init				(APP_AUDIO_DMA_DMAMUX);			// enable DMAMUX clock

	EDMA_GetDefaultConfig	(&dmaConfig);   				// get default EDMA config

	EDMA_Init				(APP_AUDIO_DMA_BASE_ADDRESS,	// init EDMA with dmaConfig config
  							&dmaConfig);

	/*******************************************
	 * Init software general part
	 */
	/* Create event group for audio processing task */
	p_audioWorkFlow_handle->processEventGroup				 = xEventGroupCreate();
	p_audioWorkFlow_handle->currentProcessEventGroup_wait	 = p_audioWorkFlow_handle->processEventPing; // wait tx ping event at start

	/********************************************
	 * Delete task
	 */
	/*if (retStatus == kStatus_Success)
	{
		retStatusFunc = AUDIO_deleteTask ( p_audioDefinition_handle );
		if (retStatusFunc != kStatus_Success)
		{
			PRINTF("FAIL: Fail to delete Tx / Rx task\r\n");
			return kStatus_Fail;
		}
	}*/

	/*******************************************
	 * Init hardware Tx path
	 */
	if (retStatus == kStatus_Success)
	{
		retStatusFunc = initAudioTxPathHw( p_audioDefinition_handle );
		if (retStatusFunc != kStatus_Success)
		{
			PRINTF("FAIL: Fail to init hardware Tx path\r\n");
			return kStatus_Fail;
		}
	}

	/*******************************************
	 * Init hardware Rx path
	 */
	if (retStatus == kStatus_Success)
	{
		retStatusFunc = initAudioRxPathHw( p_audioDefinition_handle );
		if (retStatusFunc != kStatus_Success)
		{
			PRINTF("FAIL: Fail to init hardware Rx path\r\n");
			return kStatus_Fail;
		}
	}

	/*******************************************
	 * Init audio codec
	 */
	if (retStatus == kStatus_Success)
	{
		retStatusFunc = initAudioCodec( &p_audioDefinition_handle->audioTxPath_handle,
										&p_audioDefinition_handle->audioRxPath_handle
									   );
		if (retStatusFunc != kStatus_Success)
		{
			PRINTF("FAIL: Fail to init audio codec Tx/Rx path\r\n");
			return kStatus_Fail;
		}
	}

	/*******************************************
	 * USB Rx/Tx init
	 * 	 - Launch USB audio line in & out
	 *   - Streaming is managed by IT
	 */
	if (retStatus == kStatus_Success)
	{

		if (    (p_audioTxPath_handle->configParam.sink    == AUDIO_SINK_USB1)
			 || (p_audioTxPath_handle->configParam.sink    == AUDIO_SINK_USB4)
			 || (p_audioRxPath_handle->configParam.source  == AUDIO_SRC_USB2)
			 || (p_audioRxPath_handle->configParam.source  == AUDIO_SRC_USB1)
		   )
		{
			 PRINTF("Init USB composite stream (NXP AUDIO+COM): USB Audio Rx/Tx and USB Serial port Com \r\n");

			// set parameters for the circular buffer TX
			cbParamTX.bBufferSizeInBytes = APP_USB_TX_CIRCULAR_BUFFER_SIZE_BYTE;
			cbParamTX.p_cbBuffer         = g_USBCircularBufferTX;
			cbParamTX.cbChannelNum		 = g_appHandle.usbTxRx_handle.USBTxchannelNumber;
			cbParamTX.cbSampleWidth		 = p_audioTxPath_handle->configParam.bitPerSample/8;
			// Init USB TX Circular buffer struct
			retStatusFunc = toolsCircularBufferStructInit(&g_appHandle.usbTxRx_handle.cirBuffTx,
														  cbParamTX);
			if (retStatusFunc != kStatus_Success)
			{
				PRINTF("FAIL: Fail to init TX USB circular buffer structure\r\n");
				return kStatus_Fail;
			}

			cbParamRX.bBufferSizeInBytes = APP_USB_RX_CIRCULAR_BUFFER_SIZE_BYTE;
			cbParamRX.p_cbBuffer         = &g_USBCircularBufferRX[0];
			cbParamRX.cbChannelNum		 = g_appHandle.usbTxRx_handle.USBRxchannelNumber;
			cbParamRX.cbSampleWidth		 = g_appHandle.usbTxRx_handle.USBRxBitDepth /8;
			// Init USB RX Circular buffer struct
			retStatusFunc = toolsCircularBufferStructInit(&g_appHandle.usbTxRx_handle.cirBuffRx,
														  cbParamRX);
			if (retStatusFunc != kStatus_Success)
			{
				PRINTF("FAIL: Fail to init RX USB circular buffer structure\r\n");
				return kStatus_Fail;
			}

			USB_Init(&g_appHandle.usbTxRx_handle);
		}
	}

	return retStatus;
}
