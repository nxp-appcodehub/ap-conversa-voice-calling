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
#include "fsl_debug_console.h"

//Application headers
#include "appGlobal.h"





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

shell_status_t shellVersion(shell_handle_t shellHandle, int32_t argc, char **argv)
{

	PRINTF("\n\tAudio use case version: %d.%d.%d\r\n\n",APP_VERSION_MAJOR,APP_VERSION_MINOR,APP_VERSION_PATCH);
    PRINTF("\tCode compiled for platform: %s\r\n\n",APP_PLATFORM_TEXT);
#if MIPS_MEASURE_GPIO==1
	PRINTF("\tMIPS measure by GPIO is present\r\n\n");
	PRINTF("\t\t%s\r\n",APP_GPIO_0_PURPOSE_TEXT);
	PRINTF("\t\t%s\r\n",APP_GPIO_1_PURPOSE_TEXT);
	PRINTF("\t\t%s\r\n",APP_GPIO_3_PURPOSE_TEXT);
	PRINTF("\t\t%s\r\n",APP_GPIO_4_PURPOSE_TEXT);
#else
	PRINTF("\tNo MIPS measure by GPIO\r\n\n");
#endif

    return kStatus_SHELL_Success;
}



