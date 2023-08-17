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
// fsl general purpose
#include "fsl_common.h"
#include "fsl_mailbox.h"
#include "Core0MCMGR.h"
#include "swProcessConversa.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define MSB_MASK 		0xFFFF0000
#define LSB_MASK 		0x0000FFFF
#define MSB_SHIFT 		16
#define CPU_ID    		kMAILBOX_CM33_Core0


/*******************************************************************************
 * Variables
 ******************************************************************************/
extern PL_UINT32  g_sharedAddress;

/*******************************************************************************
 * Code
 ******************************************************************************/

/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            callBack_MCMGR_Core0_Reset								 				 */
/*                                                                                               */
/* DESCRIPTION:    	    core1 will ask core0 to do a SW Reset                                    */
/* PARAMETERS:          eventData             Transmitted event data                             */
/*						userData              User Data                                          */
/*                                                                                               */
/*************************************************************************************************/
void callBack_MCMGR_Core0_Reset(uint16_t eventData, void * userData)
{
	__disable_irq();
	NVIC_SystemReset();
	while(1){};


}

/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            callBack_MCMGR_Core0_AskConversaCreateEvent								 */
/*                                                                                               */
/* DESCRIPTION:    	    core1 will ask core0 to Create Conversa                                  */
/* PARAMETERS:          eventData             Transmitted event data                             */
/*						userData              User Data                                          */
/*                                                                                               */
/*************************************************************************************************/
void callBack_MCMGR_Core0_AskConversaCreateEvent(uint16_t eventData, void * userData)
{
	status_t retStatus;
	// Conversa create function
	retStatus = initSetConversa((void*)g_sharedAddress);						//g_sharedAddress is the address of the shared block CommonVarSharedByCore0AndCore1
	// Clear event
	MAILBOX_ClearValueBits(MAILBOX,
						   CPU_ID,
						   CLEAR_VALUE);
	// Conversa is Init => we send the info to Core1
	MCMGR_TriggerEvent(kMCMGR_Core1_AnswerConversaCreate, (uint16_t) retStatus);

}

/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            callBack_MCMGR_Core0_AskConversaProcessEvent							 */
/*                                                                                               */
/* DESCRIPTION:    	    core1 will ask core0 to process 1 frame in Conversa                      */
/* PARAMETERS:          eventData             Transmitted event data                             */
/*						userData              User Data                                          */
/*                                                                                               */
/*************************************************************************************************/
void callBack_MCMGR_Core0_AskConversaProcessEvent(uint16_t eventData, void * userData)
{
	status_t retStatus;
	// Clear event
	MAILBOX_ClearValueBits(MAILBOX,
						   CPU_ID,
						   CLEAR_VALUE);
	//Conversa Process function
#ifdef MIPS_MEASURE_GPIO
	GPIO_1_Up();
#endif
	retStatus = swProcessConversa((void *)g_sharedAddress);	 //g_sharedAddress is the address of the shared block CommonVarSharedByCore0AndCore1
	if(retStatus != kStatus_Success)
	{
		MCMGR_TriggerEvent(kMCMGR_Core1_AnswerConversaProcess, (uint16_t) retStatus);
	}
#ifdef MIPS_MEASURE_GPIO
	GPIO_1_Dn();
#endif

}

/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            callBack_MCMGR_Core0_ReceiveSharedAddressMSB							 */
/*                                                                                               */
/* DESCRIPTION:    	    core1 send Shared struct address MSB to core0                            */
/* PARAMETERS:          eventData             Transmitted event data                             */
/*						userData              User Data                                          */
/*                                                                                               */
/*************************************************************************************************/
void callBack_MCMGR_Core0_ReceiveSharedAddressMSB(uint16_t eventData, void * userData)
{

	// Assign MSB to g_shared address
	g_sharedAddress &= ((PL_UINT32)eventData) << MSB_SHIFT ;
	// Clear event
	MAILBOX_ClearValueBits(MAILBOX,
						   CPU_ID,
						   CLEAR_VALUE);
}

/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            callBack_MCMGR_Core0_ReceiveSharedAddressLSB							 */
/*                                                                                               */
/* DESCRIPTION:    	    core1 send Shared struct address LSB to core0                            */
/* PARAMETERS:          eventData             Transmitted event data                             */
/*						userData              User Data                                          */
/*                                                                                               */
/*************************************************************************************************/
void callBack_MCMGR_Core0_ReceiveSharedAddressLSB(uint16_t eventData, void * userData)
{
	// Assign MSB to g_shared address
	g_sharedAddress |= (PL_UINT32)(eventData);
	// Clear event
	MAILBOX_ClearValueBits(MAILBOX,
						   CPU_ID,
						   CLEAR_VALUE);
}
/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            CORE0_initMCMGR							 								 */
/*                                                                                               */
/* DESCRIPTION:    	    Init MCMGR on Core0 + assigned user event                                */
/* PARAMETERS:                                        											 */
/*                                                                                               */
/*************************************************************************************************/
status_t CORE0_initMCMGR(void)
{
	mcmgr_status_t retFuncStatus;
	/* register User defined event */
	/*
	 * User event : kMCMGR_Core0_Reset
	 * 				core1 will ask core0 to Reset
	 */
	retFuncStatus =  MCMGR_RegisterEvent(kMCMGR_Core0_Reset,
										 callBack_MCMGR_Core0_Reset,
										 (void *)PL_NULL);
	if (retFuncStatus != kStatus_MCMGR_Success)
	{
		return kStatus_Fail;
}
	/*
	 * User event : kMCMGR_Core0_AskConversaCreate
	 * 				core1 will ask core0 to Create Conversa
	 */
	retFuncStatus =  MCMGR_RegisterEvent(kMCMGR_Core0_AskConversaCreate,
										 callBack_MCMGR_Core0_AskConversaCreateEvent,
										 (void *)PL_NULL);
	if (retFuncStatus != kStatus_MCMGR_Success)
	{
		return kStatus_Fail;
	}
	/*
	 * User event : kMCMGR_Core1AskConversaProcess
	 * 				core1 will ask core0 to process 1 frame in Conversa
	 */
	retFuncStatus =  MCMGR_RegisterEvent(kMCMGR_Core0_AskConversaProcess,
			                             callBack_MCMGR_Core0_AskConversaProcessEvent,
										 (void *)PL_NULL);
	if (retFuncStatus != kStatus_MCMGR_Success)
	{
		return kStatus_Fail;
	}
	/*
	 * User event : kMCMGR_Core0_ReceiveSharedAddressLSB
	 * 				core1 send Shared struct address LSB to core0
	 */
	retFuncStatus =  MCMGR_RegisterEvent(kMCMGR_Core0_ReceiveSharedAddressLSB,
										 callBack_MCMGR_Core0_ReceiveSharedAddressLSB,
										 (void *)PL_NULL);
	if (retFuncStatus != kStatus_MCMGR_Success)
	{
		return kStatus_Fail;
	}
	/*
	 * User event : kMCMGR_Core0_ReceiveSharedAddressMSB
	 * 				core1 send Shared struct address MSB to core0
	 */
	retFuncStatus =  MCMGR_RegisterEvent(kMCMGR_Core0_ReceiveSharedAddressMSB,
										 callBack_MCMGR_Core0_ReceiveSharedAddressMSB,
										 (void *)PL_NULL);
	if (retFuncStatus != kStatus_MCMGR_Success)
	{
		return kStatus_Fail;
	}
	/* Initialize MCMGR, install generic event handlers */
	(void)MCMGR_Init();

	return kStatus_Success;
}
/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            CORE0_checkSharedStrucrAddress							 				 */
/*                                                                                               */
/* DESCRIPTION:    	   Ensure that sharedStruct address has been well received                   */
/* PARAMETERS:                                        											 */
/*                                                                                               */
/*************************************************************************************************/
PL_BOOL CORE0_checkSharedStrucrAddress(void)
{
	// Check that MSB , LSB are both  updated
	// return 0 if MSB & LSB not both updated
	// return 1 if both updated
	return(PL_BOOL) (  ((g_sharedAddress >> MSB_SHIFT) != LSB_MASK )
			         &&((g_sharedAddress << MSB_SHIFT) != MSB_MASK ));
}


