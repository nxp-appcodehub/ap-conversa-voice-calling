/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

//Standard headers
#include <stdint.h>

//Platform headers

//fsl driver headers
#include "fsl_shell.h"
#include "fsl_mailbox.h"
#include "fsl_debug_console.h"

//Application headers
#include "appGlobal.h"
#include "mcmgr.h"
#include "Core1MCMGR.h"




/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
extern volatile T_CommonVarSharedByCore0AndCore1 CommonVarSharedByCore0AndCore1;

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

shell_status_t shellReset(shell_handle_t shellHandle, int32_t argc, char **argv)
{
	MCMGR_TriggerEvent(kMCMGR_Core0_Reset, (uint16_t) 0);
    return kStatus_SHELL_Success;
}


