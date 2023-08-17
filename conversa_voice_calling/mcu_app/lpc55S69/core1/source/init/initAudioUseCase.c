/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

//Standard headers

//Platform headers
#include <init.h>
#include "board.h"

//fsl driver headers
#include "fsl_debug_console.h"
#include "fsl_dma.h"
#include "fsl_i2s.h"
#include "fsl_i2s_dma.h"
#include "fsl_sysctl.h"

//Application headers
#include "appGlobal.h"
#include "IRQ_callBack.h"

// USB
#include "usb_init.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
extern volatile T_CommonVarSharedByCore0AndCore1 CommonVarSharedByCore0AndCore1;

/*******************************************************************************
 * Code
 ******************************************************************************/

status_t AUDIO_SetHandleDefault(AUDIO_definition_st* p_audioHandle)
{
	PL_INT8 i = 0;
	if (p_audioHandle == NULL)
	{
		return kStatus_NullPointer;
	}
	// flush the audio handle
    memset(p_audioHandle,0,sizeof(AUDIO_definition_st));
	// RxPATH
#if RX_PATH_PRESENT
	p_audioHandle->audioRxPath_handle.operatingMode				  		 = AUDIO_OM_DISABLE;
	p_audioHandle->audioRxPath_handle.configParam.source	      		 = AUDIO_SRC_NO;
	p_audioHandle->audioRxPath_handle.configParam.sink            		 = AUDIO_SINK_NO;
	p_audioHandle->audioRxPath_handle.configParam.sampleRate      		 = 16000;
	p_audioHandle->audioRxPath_handle.configParam.samplePerFrame  		 = 64;
	p_audioHandle->audioRxPath_handle.configParam.bitPerSample    		 = 32;
	p_audioHandle->audioRxPath_handle.configParam.channelNumber   		 = 2;
	p_audioHandle->audioRxPath_handle.configParam.bytePerFrame    		 = p_audioHandle->audioRxPath_handle.configParam.samplePerFrame * p_audioHandle->audioRxPath_handle.configParam.bitPerSample / 8 * p_audioHandle->audioRxPath_handle.configParam.channelNumber ;
	p_audioHandle->audioRxPath_handle.configParam.bytePerFramePerChannel = p_audioHandle->audioRxPath_handle.configParam.samplePerFrame * p_audioHandle->audioRxPath_handle.configParam.bitPerSample / 8;
#endif
	// TxPATH
	p_audioHandle->audioTxPath_handle.operatingMode				  		 = AUDIO_OM_DISABLE;
	p_audioHandle->audioTxPath_handle.configParam.source	      		 = AUDIO_SRC_NO;
	p_audioHandle->audioTxPath_handle.configParam.sink            		 = AUDIO_SINK_NO;
	p_audioHandle->audioTxPath_handle.configParam.sampleRate      		 = 16000;
	p_audioHandle->audioTxPath_handle.configParam.samplePerFrame  		 = 64;
	p_audioHandle->audioTxPath_handle.configParam.bitPerSample    		 = 32;
	p_audioHandle->audioTxPath_handle.configParam.channelNumber   		 = 2;
	p_audioHandle->audioTxPath_handle.configParam.bytePerFrame    		 = p_audioHandle->audioTxPath_handle.configParam.samplePerFrame * p_audioHandle->audioTxPath_handle.configParam.bitPerSample / 8 * p_audioHandle->audioTxPath_handle.configParam.channelNumber ;
	p_audioHandle->audioTxPath_handle.configParam.bytePerFramePerChannel = p_audioHandle->audioTxPath_handle.configParam.samplePerFrame * p_audioHandle->audioTxPath_handle.configParam.bitPerSample / 8;
    // State variables
	p_audioHandle->audioPathIsDefined									 = PL_FALSE;
	p_audioHandle->audioPathIsInit										 = PL_FALSE;

	return kStatus_Success;
}
status_t initAudioUseCase(AUDIO_definition_st* p_audioDefinition_handle)
{
	// error handle
	status_t retStatus 	   = kStatus_Success;
	status_t retStatusFunc = kStatus_Success;

	AUDIO_definitionPath_st* p_audioRxPath_handle 	   = &p_audioDefinition_handle->audioRxPath_handle;
	AUDIO_definitionPath_st* p_audioTxPath_handle 	   = &p_audioDefinition_handle->audioTxPath_handle;
	AUDIO_workFlowHandle_st* p_audioWorkFlow_handle    = &p_audioDefinition_handle->audioWorkFlow_handle;
	TOOLS_circularBuffer_param cbParamRX;
	TOOLS_circularBuffer_param cbParamTX;
	if (p_audioDefinition_handle == NULL)
	{
		return kStatus_NullPointer;
	}
	/*
	* INIT USB
	*
	*/
	InitUsbCompositeDevice();
	/*******************************************
	 * Init hardware general part
	 */
    DMA_Init(APP_AUDIO_DMA_BASE_ADDRESS);
    initFlexcommPinsSharing(&g_appHandle.audioDefinition);

	/*******************************************
	 * Init software general part
	 */
    // First buffer to be loaded by I2S is always PING so the process will first wait for PING
    p_audioWorkFlow_handle->pingPongTxStatus 				=  AUDIO_WF_PONG;
#if RX_PATH_PRESENT
    p_audioWorkFlow_handle->pingPongRxStatus 				=  AUDIO_WF_PONG;
#endif
    p_audioWorkFlow_handle->currentProcessEventGroup_wait   =  p_audioWorkFlow_handle->processEventPing;

	/*******************************************
	 * Init hardware Tx path
	 */
#if RX_PATH_PRESENT
    initAudioRxPathHw(p_audioDefinition_handle);
#endif
	/*******************************************
	 * Init hardware Rx path
	 */

    initAudioTxPathHw(p_audioDefinition_handle);

    /*******************************************
	 * USB Rx/Tx init
	 * 	 - Launch USB audio line in & out
	 *   - Streaming is managed by IT
	 */

	if (    (p_audioTxPath_handle->configParam.sink    == AUDIO_SINK_USB1)
		 || (p_audioTxPath_handle->configParam.sink    == AUDIO_SINK_USB4)
		 || (p_audioRxPath_handle->configParam.source  == AUDIO_SRC_USB2)
		 || (p_audioRxPath_handle->configParam.source  == AUDIO_SRC_USB1)
	   )
		{

			// set parameters for the circular buffer TX
			cbParamTX.bBufferSizeInBytes = APP_USB_TX_CIRCULAR_BUFFER_SIZE_BYTE;
			cbParamTX.p_cbBuffer         = g_USBCircularBufferTX;
			cbParamTX.cbChannelNum		 = g_appHandle.UsbCompositeDev_handle.AUDIO_IN_FORMAT_CHANNELS;
			cbParamTX.cbSampleWidth		 = p_audioTxPath_handle->configParam.bitPerSample/8;
			// Init USB TX Circular buffer struct
			retStatusFunc = toolsCircularBufferStructInit(&g_appHandle.UsbCompositeDev_handle.cirBuffTx,
														  cbParamTX);
			if (retStatusFunc != kStatus_Success)
			{
				PRINTF("FAIL: Fail to init TX USB circular buffer structure\r\n");
				return kStatus_Fail;
			}
#if RX_PATH_PRESENT
			cbParamRX.bBufferSizeInBytes = APP_USB_RX_CIRCULAR_BUFFER_SIZE_BYTE;
			cbParamRX.p_cbBuffer         = &g_USBCircularBufferRX[0];
			cbParamRX.cbChannelNum		 = g_appHandle.UsbCompositeDev_handle.AUDIO_OUT_FORMAT_CHANNELS;
			cbParamRX.cbSampleWidth		 = g_appHandle.UsbCompositeDev_handle.AUDIO_OUT_FORMAT_SIZE ;
			// Init USB RX Circular buffer struct
			retStatusFunc = toolsCircularBufferStructInit(&g_appHandle.UsbCompositeDev_handle.cirBuffRx,
														  cbParamRX);
			if (retStatusFunc != kStatus_Success)
			{
				PRINTF("FAIL: Fail to init RX USB circular buffer structure\r\n");
				return kStatus_Fail;
			}
#endif
		}
	/*******************************************
	 * Init hardware CODEC
	 */
#if RX_PATH_PRESENT
	initCodecWM8904();
#endif
    g_appHandle.audioDefinition.audioPathIsInit =1;

	return retStatus;
}
