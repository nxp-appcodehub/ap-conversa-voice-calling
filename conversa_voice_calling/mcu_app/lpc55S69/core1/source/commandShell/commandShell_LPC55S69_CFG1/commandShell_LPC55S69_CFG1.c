/*
 * Copyright (c) 2015 - 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2017,2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


//Standard headers
#include <init.h>
#include <stdint.h>

//Platform headers
#include "board.h"

//fsl driver headers
#include "fsl_debug_console.h"
#include "fsl_mailbox.h"
#include "fsl_component_serial_manager.h"
#include "fsl_shell.h"

//Application headers
#include "appGlobal.h"
#include "commandShell_LPC55S69_CFG1.h"



/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static shell_status_t LedControl(shell_handle_t shellHandle, int32_t argc, char **argv);
extern serial_handle_t g_serialHandle;
/*******************************************************************************
 * Variables
 ******************************************************************************/
TSupportedShellCmd CurrentShellCmd;


/***************** COMMAND SHELL DEFINITION FOR ALL PLATFORM ****************/
SHELL_COMMAND_DEFINE(version,
		             "\r\n\"version\": Display component versions\r\n",
					 shellVersion,
					 0);

SHELL_COMMAND_DEFINE(reset,
		             "\r\n\"reset\": Software reset\r\n",
					 shellReset,
					 0);

/***************** COMMAND SHELL DEFINITION FOR DIFFERENT PLATFORM ****************/


SHELL_COMMAND_DEFINE(voicecall,
		"\r\n\"voicecall\": type run \"voicecall [pre-Load configuration]\" to run a pre-Load configuration voice scenario\r\n\n"
				"\tPre-load configuration available (command line):\r\n\n"
					 "\n"
					 /* begin SOFTWARE PACK CONFIGURATION */
					 "\t\tvoicecall spswp16k => Speaker phone with Conversa software (required voice call LPC55S69 Mockup). Teams conferencing device 1.5m. Fs=16kHz.\r\n"
					 "\n"
					 "\t\tvoicecall usbswp16k => Usb microphones and Usb speaker. Fs=16kHz\r\n"
					 "\n"
					 "\t\tvoicecall lbswp16k => Loop back between microphones and headset + Usb microphones. Fs=16kHz\r\n"
					 "\n"
					 /* end SOFTWARE PACK CONFIGURATION */
					 /* begin TO BE REMOVED FOR RELEASE */
					 /* begin NXP INTERNAL CONFIGURATION */
					 /* end NXP INTERNAL CONFIGURATION */
					 /* begin CUSTOMER CONFIGURATION */
					 /* end CUSTOMER CONFIGURATION */
					 /* begin TEST AND DEBUG CONFIGURATION */
					 /* end TEST AND DEBUG CONFIGURATION */
					 /* end TO BE REMOVED FOR RELEASE */
					 /* General to not delete */
			"\tLegend:\r\n\n"
					"\t\tLPC55S69 mockup : LPC55S69 EVK (2 microphones used) + TA2024 amplifier + 1 TangBand 2136 speaker\r\n\n"
					 "\t\tsp: sp means speaker phone with Conversa software. Conversa Tx output is sent to Tx Usb channel 0. Conversa Rx output is sent to the speaker\r\n\n"
					  "\t\tlb  : lb means loop back. Tx chain input are sent to speakers and to Usb Tx. Usb Rx datas are not handled\r\n\n"
					 "\t\tusb : usb only use case, no process is running. Tx chain input (microphones) datas are sent Usb Tx. Usb Rx datas are sent to speakers. No process is present\r\n\n"
					 "\n"
			"\tWARNING : FOR lb CONFIGURATION REMOVE SPEAKER JACK CONNECTOR AND PLUG A HEADSET TO AVOID HOWLING AND POTENTIAL SPEAKER DAMAGE: WARNING\r\n"
					 "\n"
					 "\n",
                     shellTxRxPathDef,
                     SHELL_IGNORE_PARAMETER_COUNT);



SDK_ALIGN(static uint8_t s_shellHandleBuffer[SHELL_HANDLE_SIZE], 4);
shell_handle_t s_shellHandle;


/*******************************************************************************
 * Code
 ******************************************************************************/
void InitShell(void)
{
    s_shellHandle = &s_shellHandleBuffer[0];

    SHELL_Init(s_shellHandle, g_serialHandle, "");
    /* Add new command to commands list */
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(version));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(voicecall));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(reset));
}




