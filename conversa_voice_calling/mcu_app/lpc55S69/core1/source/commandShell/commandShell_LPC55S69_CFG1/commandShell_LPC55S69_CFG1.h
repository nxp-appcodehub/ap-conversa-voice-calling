/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016,2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __Shell_H__
#define __Shell_H__ 1



//#define SHELL_Printf PRINTF



typedef enum _SupportedShellCmd
{
    Cmd_None,
    Cmd_Reset,
    Cmd_Help,
    Cmd_Version,
    Cmd_Exit,
    Cmd_SetMic,
    Cmd_SetUac,
    Cmd_Voicecall,
} TSupportedShellCmd;

extern TSupportedShellCmd CurrentShellCmd;

extern void InitShell(void);
extern shell_status_t shellVersion      (shell_handle_t shellHandle, int32_t argc, char **argv);
extern shell_status_t shellReset        (shell_handle_t shellHandle, int32_t argc, char **argv);
extern shell_status_t shellTxRxPathDef  (shell_handle_t shellHandle, int32_t argc, char **argv);

#endif


