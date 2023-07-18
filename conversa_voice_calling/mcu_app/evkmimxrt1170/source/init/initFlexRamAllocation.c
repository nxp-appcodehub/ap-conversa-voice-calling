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
/* FUNCTION:            initFlexRamAllocation										             */
/*                                                                                               */
/* DESCRIPTION: reallocate flexram (DTCM,ITCM,OCRAM                                   			 */
/*               please make sure it is allign with project settings							 */
/* PARAMETERS:                                                                                   */
/*************************************************************************************************/
status_t initFlexRamAllocation(flexram_allocate_ram_t* p_ramAllocate)
{
	// error handle
	status_t 	 			 retStatus 	 	 = kStatus_Success;
	status_t 	 			 retStatusFunc   = kStatus_Success;

    retStatusFunc = FLEXRAM_AllocateRam(p_ramAllocate);
    if (retStatusFunc != kStatus_Success)
    {
    	retStatus = retStatusFunc;
    }
    return retStatus;
}
