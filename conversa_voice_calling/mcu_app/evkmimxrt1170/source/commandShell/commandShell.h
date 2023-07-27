/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __COMMAND_SHELL_H__
#define __COMMAND_SHELL_H__


#include "fsl_shell.h"

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

extern shell_status_t shellVersion      (shell_handle_t shellHandle, int32_t argc, char **argv);
extern shell_status_t shellReset        (shell_handle_t shellHandle, int32_t argc, char **argv);
extern shell_status_t shellTxRxPathDef  (shell_handle_t shellHandle, int32_t argc, char **argv);

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*${macro:start}*/
typedef void handleShellMessageCallback_t(void *arg);
/*${macro:end}*/

/*!
 * @brief Common function for getting user input using shell console.
 *
 * @param[in] handleShellMessageCallback Callback to function which should
 * handle serialized message.
 * @param[in] arg Data to pass to callback handler.
 */
void shellCmd();
/*${prototype:end}*/

#endif // __COMMAND_SHELL_H__
