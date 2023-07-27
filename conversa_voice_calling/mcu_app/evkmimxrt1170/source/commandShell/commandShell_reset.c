/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include <stdint.h>

//shell
#include "commandShell.h"
#include "fsl_shell.h"

//debug
#include "fsl_debug_console.h"

//application
#include "appGlobal.h"

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

shell_status_t shellReset(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    // disable IRQ then reset
	__disable_irq();
	NVIC_SystemReset();
    return kStatus_SHELL_Success;
}

