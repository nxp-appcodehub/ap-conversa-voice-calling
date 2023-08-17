/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


//MCMGR headers
#include "mcmgr.h"
#include "mcmgr_internal_core_api.h"
//Application headers
#include "appGlobal.h"

// fsl drivers
#include "fsl_common.h"
#include "fsl_mailbox.h"
#include "Core1MCMGR.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define MSB_MASK 		0xFFFF0000
#define LSB_MASK 		0x0000FFFF
#define CPU_ID    		kMAILBOX_CM33_Core1

/*******************************************************************************
 * Variables
 ******************************************************************************/

volatile PL_BOOL						  g_ConversaIsInit = PL_FALSE;
volatile PL_BOOL						  g_ConversaProcessError = PL_FALSE;
/*******************************************************************************
 * Code
 ******************************************************************************/
/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            callBack_MCMGR_Core1_AnswerConversaProcess							 	 */
/*                                                                                               */
/* DESCRIPTION:    	    error manager for swProcessConversa        							 	 */
/* PARAMETERS:          eventData             Transmitted event data                             */
/*						userData              User Data                                          */
/*************************************************************************************************/
static void callBack_MCMGR_Core1_AnswerConversaProcess(uint16_t eventData, void * userData)
{
	status_t ConversaProcessRetStatus = (status_t)eventData;
	// Error returned by swProcessConversa()
	if (ConversaProcessRetStatus == kStatus_NullPointer)
	{
		PRINTF("[swProcess] FAIL: null pointer \r\n");

	}
	else if (ConversaProcessRetStatus == kStatus_Fail)
	{
		PRINTF("[swProcess] FAIL: ConversaProcess() fail \r\n");

	}
	else if (ConversaProcessRetStatus == kStatus_LicenseError)
	{
		PRINTF("[swProcess] FAIL: License error \r\n");

	}
	// clear event
	MAILBOX_ClearValueBits(MAILBOX,
						  CPU_ID,
						  CLEAR_VALUE);
	// Set global to true
	g_ConversaProcessError = PL_TRUE;
}
/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            callBack_MCMGR_Core1_AnswerConversaCreate							 	 */
/*                                                                                               */
/* DESCRIPTION:    	    Core0 send answer to Core1 than ConversaCreate has been executed         */
/* PARAMETERS:          eventData             Transmitted event data                             */
/*						userData              User Data                                          */
/*************************************************************************************************/
static void callBack_MCMGR_Core1_AnswerConversaCreate(uint16_t eventData, void * userData)
{
	status_t ConversaCreateRetStatus = (status_t)eventData;
	// we can give the info that conversa is init
	if (ConversaCreateRetStatus ==kStatus_Success)
	{
		g_ConversaIsInit = PL_TRUE;
	}
	else
	{
		g_ConversaIsInit = PL_FALSE;
	}
	// clear event
	MAILBOX_ClearValueBits(MAILBOX,
						  CPU_ID,
						  CLEAR_VALUE);
}
/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            CORE1_sendSharedStructAddress							 	             */
/*                                                                                               */
/* DESCRIPTION:    	    Core1 send SharedStructAddress to Core0. Due to 16bit transfer           */
/*                      limitation, the address will be sent in 2 parts ( 16 MSB + 16 LSB)       */
/* PARAMETERS:          SharedStructAddress             TAddress to be sent                      */
/*													                                             */
/*************************************************************************************************/
status_t CORE1_sendSharedStructAddress(PL_UINT32 SharedStructAddress)
{
	// Split the address in 2 16-bit value low significant bit + Most significant bit
	PL_UINT16 MSB =  (SharedStructAddress & MSB_MASK)>>16;
	PL_UINT16 LSB =  (SharedStructAddress & LSB_MASK);
	// Send the address in 2 parts
	MCMGR_TriggerEvent(kMCMGR_Core0_ReceiveSharedAddressMSB, (uint16_t) MSB);
	MCMGR_TriggerEvent(kMCMGR_Core0_ReceiveSharedAddressLSB, (uint16_t) LSB);
	return kStatus_Success;
}
/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            CORE1_initMCMGR							 								 */
/*                                                                                               */
/* DESCRIPTION:    	    Init MCMGR on Core1 + assigned user event                                */
/* PARAMETERS:                                        											 */
/*                                                                                               */
/*************************************************************************************************/
status_t CORE1_initMCMGR(void)
{
	mcmgr_status_t retFuncStatus;
	PL_UINT32      startupData;

	/* register User defined event */
	/*
	 * User event : kMCMGR_Core1_AnswerConversaCreate
	 * 				Core0 send answer to Core1 than ConversaCreate() has been executed
	 */
	retFuncStatus =  MCMGR_RegisterEvent(kMCMGR_Core1_AnswerConversaCreate,
										 callBack_MCMGR_Core1_AnswerConversaCreate,
										 (void *)PL_NULL);
	/*
	 * User event : kMCMGR_Core1_AnswerConversaProcess
	 * 				Core0 return an error from swProcess, it will be managed by core1
	 */
	retFuncStatus =  MCMGR_RegisterEvent(kMCMGR_Core1_AnswerConversaProcess,
										 callBack_MCMGR_Core1_AnswerConversaProcess,
										 (void *)PL_NULL);

	if (retFuncStatus != kStatus_MCMGR_Success)
	{
		return kStatus_Fail;
	}
	/* Initialize MCMGR, install generic event handlers */
	(void)MCMGR_Init();
	/* Get the startup data */
	do
	{
		retFuncStatus = MCMGR_GetStartupData(&startupData);
	} while (retFuncStatus != kStatus_MCMGR_Success);


	return kStatus_Success;

}

