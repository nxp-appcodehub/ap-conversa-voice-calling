/*
 * Copyright 2020-2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __AUDIO_H__
#define __AUDIO_H__

#ifdef CORE0
#define CONVERSA_PRESENT
#endif

// fsl general purpose
#include "fsl_common.h"
// debug
#include "fsl_debug_console.h"

// Platform
#include "PL_platformTypes.h"

// Sw Ip: Conversa
#ifdef CONVERSA_PRESENT
 #include "RdspConversaPlugin.h"
 #include "RdspConversaPluginConfig.h"
#include "ConversaTuningConfig.h"
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define AUDIO_PROCESS_LIST_MAX      1 							// maximum number of process that can be called for RX or TX path
#define AUDIO_CHANNEL_NUMBER_MAX    2							// maximal audio channel supported

/*******************************************************************************
 * Enum
 ******************************************************************************/
#ifdef CONVERSA_PRESENT
	// Operating mode
	typedef enum
	{
		CONVERSA_DISABLE = 0,
		CONVERSA_ENABLE  = 1,
	}CONVERSA_operatingMode_en;

	// data format type
	typedef enum
	{
		CONVERSA_PROCESS_FLOAT,
		CONVERSA_PROCESS_Q31,
	} CONVERSA_processFormat_en;
#endif

// Operating mode
typedef enum
{
    AUDIO_OM_DISABLE,
	AUDIO_OM_ENABLE,
}AUDIO_operatingMode_en;

// audio flow direction
typedef enum
{
    AUDIO_DIRECTION_FROM_SOURCE,
	AUDIO_DIRECTION_TO_SINK,
}AUDIO_direction_en;

/* All audio process element supported by the application */
typedef enum
{
	AUDIO_PROC_NO,
	AUDIO_PROC_MIPS_5,					// simulate mips load 5%
	AUDIO_PROC_MIPS_25,					// simulate mips load 25%
	AUDIO_PROC_MIPS_50,					// simulate mips load 50%
	AUDIO_PROC_MIPS_75,					// simulate mips load 75%
	AUDIO_PROC_MIPS_95,					// simulate mips load 95%
	AUDIO_PROC_CONVERSA_TXRX,
} AUDIO_process_en;

/* All audio source input element */
typedef enum
{
	AUDIO_SRC_NO,
	AUDIO_SRC_OMNI_1DMIC,			 // 1 Digtal mic -> omni direction
	AUDIO_SRC_LINEAR_2DMIC,			 // 2 Digtal mic -> linear array
	AUDIO_SRC_TRIANGULAR_3DMIC, 	 // 3 Digtal mic -> triangular
	AUDIO_SRC_SQUARE_TRILLIUM_4DMIC, // 4 Digtal mic -> square or trillium
	AUDIO_SRC_TX0,				// Tx path channel 0
	AUDIO_SRC_TX0_1,			// Tx path channel 0 & 1
	AUDIO_SRC_USB1,				// usb audio stream 1
	AUDIO_SRC_USB2,				// usb audio stream 2
} AUDIO_sourceElement_en;

/* All audio sink output element */
typedef enum
{
	AUDIO_SINK_NO,
	AUDIO_SINK_SPEAKER1,			// speakers 1 channel
	AUDIO_SINK_SPEAKER2,			// speakers 2 channel
	AUDIO_SINK_SPEAKER1DIFF,		// speakers 1 channel differential
	AUDIO_SINK_USB1,				// usb audio stream 1 channel
	AUDIO_SINK_USB4,				// usb audio stream 4 channel
	//AUDIO_SINK_USB5,				// usb audio stream 5 channel
} AUDIO_sinkElement_en;

// ping pong event condition
typedef enum
{
	AUDIO_NO_EVENT_BUFF 	= 0,
	AUDIO_TX_I2S0_PING_BUFF = (1U << 1U),
	AUDIO_TX_I2S0_PONG_BUFF = (1U << 2U),
	AUDIO_TX_I2S1_PING_BUFF = (1U << 3U),
	AUDIO_TX_I2S1_PONG_BUFF = (1U << 4U),
	AUDIO_RX_I2S0_PING_BUFF = (1U << 5U),
	AUDIO_RX_I2S0_PONG_BUFF = (1U << 6U),

}AUDIO_RXTX_PINGPONG_CONDITION_en;

// ping pong worflow status
typedef enum
{
	AUDIO_WF_PING = 0,
	AUDIO_WF_PONG = 1,
}AUDIO_WF_PINGPONG_STATUS_en;

/*******************************************************************************
 * Structure
 ******************************************************************************/
/* Sw IP : Conversa */
#ifdef CONVERSA_PRESENT

typedef struct
{
	conversa_parameter_config_t* 			p_conversaTuningParameter;					// tuning parameter
	PL_BOOL									isThereTuningFile;							// boolean to indicates if there is a tuning file or not
} AUDIO_ConversaTuningParam_st;


typedef struct
{
	AUDIO_operatingMode_en   				operatingModeTx;				 // enable disable conversa Sw Ip Tx path
	AUDIO_operatingMode_en   				operatingModeRx;				 // enable disable conversa Sw Ip Rx path
	PL_UINT32							    conversaDataControlAddress; 	 // Address control the conversa data (used by the tuning tool)
	rdsp_conversa_plugin_constants_t		conversaPluginConst;    		 // constant which decribe the conversa limit
	rdsp_conversa_plugin_t					conversaPluginParams;   		 // conversa parameters
	AUDIO_ConversaTuningParam_st			conversaTuningParamStruct; 		 // conversa tuning parameters structure
	PL_BOOL 								conversaLicenseTimeOut;	     	 // conversa license time out False True
	PL_BOOL				    				Core0Busy;						 // State variable
} AUDIO_conversa_st;
#endif

/* Audio */
typedef struct
{
	void* 	p_pingBufferAddress[AUDIO_CHANNEL_NUMBER_MAX];
	void*	p_pongBufferAddress[AUDIO_CHANNEL_NUMBER_MAX];
} AUDIO_bufferHandle_st;

typedef struct
{
	volatile AUDIO_RXTX_PINGPONG_CONDITION_en  	currentProcessEventGroup_wait;    // edma event bit waited to continue
	volatile AUDIO_RXTX_PINGPONG_CONDITION_en  	currentProcessEventGroup_status;  // current PingPong status of process
	AUDIO_RXTX_PINGPONG_CONDITION_en  			processEventPing;				  // event assigned to ping for process
	AUDIO_RXTX_PINGPONG_CONDITION_en  			processEventPong;				  // event assigned to pong for process
	volatile AUDIO_WF_PINGPONG_STATUS_en        pingPongTxStatus;				  // ping pong status of TX I2S transfer.
	volatile AUDIO_WF_PINGPONG_STATUS_en        pingPongRxStatus;				  // ping pong status of RX I2S transfer.
} AUDIO_workFlowHandle_st;

typedef struct
{
	AUDIO_sourceElement_en 	source;
	AUDIO_sinkElement_en   	sink;
	PL_INT32			   	sampleRate;
	PL_INT32			   	samplePerFrame;
	PL_INT32			   	bitPerSample;
	PL_INT32			   	bytePerFrame;
	PL_INT32			   	bytePerFramePerChannel;
	PL_INT8			   	   	channelNumber;
	AUDIO_bufferHandle_st  	bufferHandle;
} AUDIO_configParam_st;

typedef struct
{
	AUDIO_operatingMode_en 	operatingMode;
	AUDIO_configParam_st   	configParam;
} AUDIO_definitionPath_st;

typedef struct
{
	AUDIO_workFlowHandle_st audioWorkFlow_handle;
	AUDIO_definitionPath_st audioTxPath_handle;
	AUDIO_definitionPath_st audioRxPath_handle;
	AUDIO_process_en       	processList[AUDIO_PROCESS_LIST_MAX];
	volatile PL_BOOL		audioPathIsDefined;
	volatile PL_BOOL		audioPathIsInit;
#ifdef CONVERSA_PRESENT
	AUDIO_conversa_st		swIpConversa_handle;
	PL_INT8					ConversaPresetToLoad;
	PL_BOOL				    ConfigWithConversa;


#endif
}AUDIO_definition_st;

/*******************************************************************************
 * Function
 ******************************************************************************/
//init AUDIO_definition_st to default value
status_t AUDIO_SetHandleDefault(AUDIO_definition_st* audioHandle);

#endif /* __AUDIO_H__ */
