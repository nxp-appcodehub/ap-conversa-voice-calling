/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include <stdint.h>

// Global
#include "appGlobal.h"

// shell
#include "commandShell.h"
#include "fsl_shell.h"

//debug
#include "fsl_debug_console.h"

#if (APP_PLATFORM == APP_PL_RT1170EVK)
/*******************************************************************************
 * Definitions
 ******************************************************************************/

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
					 /* SOFTWARE PACK CONFIGURATION */
					 "\t\tvoicecall spswp16k => Speaker phone with Conversa software (required voice call RT1170 mockup). Teams conferencing device 2.3m. Fs=16kHz.\r\n"
 	 	 	 	 	 "\t\tvoicecall spswp32k => Speaker phone with Conversa software (required voice call RT1170 mockup). Teams conferencing device 2.3m. Fs=32kHz.\r\n"
        			 "\n"
					 "\t\tvoicecall usbswp16k => Usb microphones and Usb speaker. Fs=16kHz\r\n"
					 "\t\tvoicecall usbswp32k => Usb microphones and Usb speaker. Fs=32kHz\r\n"
					 "\n"
					 "\t\tvoicecall lbswp16k => Loop back between microphones and headset + Usb microphones. Fs=16kHz\r\n"
        			 "\t\tvoicecall lbswp32k => Loop back between microphones and headset + Usb microphones. Fs=32kHz\r\n"
					 "\n"
					 /* end SOFTWARE PACK CONFIGURATION */

					 /* General to not delete */
			"\tLegend:\r\n\n"
					"\t\tRT1170 mockup : RT1170 EVKA (3 microphones used) + TA2024 amplifier + 1 or 2 TangBand 2136 speaker\r\n\n"
					 "\t\tsp: sp means speaker phone with Conversa software. Conversa Tx output is sent to Tx Usb channel 0. Conversa Rx output is sent to the speaker\r\n\n"
					  "\t\tlb  : lb means loop back. Tx chain input are sent to speakers and to Usb Tx. Usb Rx datas are not handled\r\n\n"
					 "\t\tusb : usb only use case, no process is running. Tx chain input (microphones) datas are sent Usb Tx. Usb Rx datas are sent to speakers. No process is present\r\n\n"
					 "\n"
			"\tWARNING : FOR lb CONFIGURATION REMOVE SPEAKER JACK CONNECTOR AND PLUG A HEADSET TO AVOID HOWLING AND POTENTIAL SPEAKER DAMAGE: WARNING\r\n"
					 "\n"
					 "\n",
					 shellTxRxPathDef,
                     SHELL_IGNORE_PARAMETER_COUNT);
#endif



/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
/* Shell task */
SDK_ALIGN(static uint8_t s_shellHandleBuffer[SHELL_HANDLE_SIZE], 4);
static shell_handle_t s_shellHandle;
extern serial_handle_t g_serialHandle;

/*******************************************************************************
 * Code
 ******************************************************************************/
void shellCmd()
{
    /* Init SHELL */
    s_shellHandle = &s_shellHandleBuffer[0];
    SHELL_Init(s_shellHandle, g_serialHandle, ">> ");

    /* Add new command to commands list */
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(version));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(reset));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(voicecall));
    // SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(run));


#if !(defined(SHELL_NON_BLOCKING_MODE) && (SHELL_NON_BLOCKING_MODE > 0U))
    while (1)
    {
        SHELL_Task(s_shellHandle);
    }
#endif
}


