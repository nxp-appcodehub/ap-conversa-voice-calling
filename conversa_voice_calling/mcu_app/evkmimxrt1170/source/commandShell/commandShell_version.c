/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include <stdint.h>

//board
#include "board.h"

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

shell_status_t shellVersion(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    // conversa variable
#ifdef CONVERSA_PRESENT
	PL_UINT32 conversaVersionMajor;
	PL_UINT32 conversaVersionMinor;
	PL_UINT32 conversaVersionPatch;

	rdsp_conversa_plugin_constants_t* p_conversaPluginConst = &g_appHandle.audioDefinition.swIpConversa_handle.conversaPluginConst;
#endif

	PRINTF("\n\tAudio frame work version: %d.%d.%d\r\n\n",APP_VERSION_MAJOR,APP_VERSION_MINOR,APP_VERSION_PATCH);
    PRINTF("\tCode compiled for platform: %s\r\n\n",APP_PLATFORM_TEXT);
	/*
     * Conversa information
     */
#ifdef CONVERSA_PRESENT
	/***********************************************/
    /* CONVERSA LIBRAY INFO */
    /* Get conversa version */
	RdspConversa_Plugin_GetLibVersion(	&g_appHandle.audioDefinition.swIpConversa_handle.conversaPluginParams,
										(uint32_t*) &conversaVersionMajor,
										(uint32_t*) &conversaVersionMinor,
										(uint32_t*) &conversaVersionPatch);

    // Read conversa library limitation
	RdspConversa_Plugin_GetConstants(p_conversaPluginConst);

	if ( p_conversaPluginConst->build_for_headset == 1)
	{
		if (	(conversaVersionMajor != 0)
			&&  (conversaVersionMinor != 0)
			&&  (conversaVersionPatch != 0)
			)
		{
			PRINTF("\tConversa library for headset v%i.%i.%i is present:\r\n\n",conversaVersionMajor,conversaVersionMinor,conversaVersionPatch);
		}
		else
		{
			PRINTF("\tConversa library for headset version unknown is present:\r\n\n");
		}
	}
	else
	{
		if (	(conversaVersionMajor != 0)
			&&  (conversaVersionMinor != 0)
			&&  (conversaVersionPatch != 0)
			)
		{
			PRINTF("\tConversa library for speaker v%i.%i.%i is present:\r\n\n",conversaVersionMajor,conversaVersionMinor,conversaVersionPatch);
		}
		else
		{
			PRINTF("\tConversa library for speaker version unknown is present:\r\n\n");
		}
	}

	PRINTF("\t\t Rx sample rate            = %i Hz\r\n"					, p_conversaPluginConst->rx_fs);
	PRINTF("\t\t Tx sample rate            = %i Hz\r\n"					, p_conversaPluginConst->tx_fs);
	PRINTF("\t\t Audio frame size          = %i samples per frame\r\n"	, p_conversaPluginConst->audio_framesize);
	PRINTF("\t\t Max input channel number  = %i\r\n"					, p_conversaPluginConst->max_num_mics);
	PRINTF("\t\t Max output channel number = %i\r\n"					, p_conversaPluginConst->max_num_spks);
	PRINTF("\t\t Audio sample bit depth    = %i bits\r\n\n"				, p_conversaPluginConst->num_bytes_per_sample*8);

	/***********************************************/
	/* CONVERSA PARAMETER FILE INFO */
	if (g_appHandle.audioDefinition.swIpConversa_handle.conversaTuningParamStruct.isThereTuningFile == PL_TRUE)
	{
		PRINTF("\tConversa parameter selected:\r\n\n");
		PRINTF("\t\t Version: %d.%d.%d\r\n",g_appHandle.audioDefinition.swIpConversa_handle.conversaTuningParamStruct.p_conversaTuningParameter->conversa_lib_version[0],
										    g_appHandle.audioDefinition.swIpConversa_handle.conversaTuningParamStruct.p_conversaTuningParameter->conversa_lib_version[1],
										    g_appHandle.audioDefinition.swIpConversa_handle.conversaTuningParamStruct.p_conversaTuningParameter->conversa_lib_version[2]);

		PRINTF("\t\t settings format version  = %d\r\n"   ,  g_appHandle.audioDefinition.swIpConversa_handle.conversaTuningParamStruct.p_conversaTuningParameter->settings_format_version);
		PRINTF("\t\t Input channel number     = %d\r\n"   ,  g_appHandle.audioDefinition.swIpConversa_handle.conversaTuningParamStruct.p_conversaTuningParameter->num_mic);
		PRINTF("\t\t Output channel numbers   = %d\r\n"   ,  g_appHandle.audioDefinition.swIpConversa_handle.conversaTuningParamStruct.p_conversaTuningParameter->num_spk);
		PRINTF("\t\t Tx sample rate           = %d Hz\r\n",  g_appHandle.audioDefinition.swIpConversa_handle.conversaTuningParamStruct.p_conversaTuningParameter->tx_samplerate);
		PRINTF("\t\t Rx sample rate           = %d Hz\r\n",  g_appHandle.audioDefinition.swIpConversa_handle.conversaTuningParamStruct.p_conversaTuningParameter->rx_samplerate);
		PRINTF("\t\t Audio frame size         = %d samples per frame\r\n",  g_appHandle.audioDefinition.swIpConversa_handle.conversaTuningParamStruct.p_conversaTuningParameter->dsp_blocksize);
		PRINTF("\t\t Filter band number       = %d\r\n"   ,  g_appHandle.audioDefinition.swIpConversa_handle.conversaTuningParamStruct.p_conversaTuningParameter->num_bands);
		PRINTF("\t\t Parameter numbers        = %d\r\n"   ,  g_appHandle.audioDefinition.swIpConversa_handle.conversaTuningParamStruct.p_conversaTuningParameter->parameter_data_size);
		PRINTF("\t\t Information              = %s\r\n\n" ,  g_appHandle.audioDefinition.swIpConversa_handle.conversaTuningParamStruct.p_conversaTuningParameter->info_str);
	}
	else
	{
		PRINTF("\tConversa parameter file not selected\r\n\n");
	}

#else
	PRINTF("\tNo Conversa process available\r\n\n");
#endif

#ifdef MIPS_MEASURE_GPIO
	PRINTF("\tMIPS measure by GPIO is present:\r\n\n");
	PRINTF("\t\t%s\r\n",BOARD_USER_MIPS_GPIO_1_PURPOSE_TEXT);
	PRINTF("\t\t%s\r\n",BOARD_USER_MIPS_GPIO_2_PURPOSE_TEXT);
	PRINTF("\t\t%s\r\n",BOARD_USER_MIPS_GPIO_3_PURPOSE_TEXT);
	PRINTF("\t\t%s\r\n",BOARD_USER_MIPS_GPIO_4_PURPOSE_TEXT);
	PRINTF("\t\t%s\r\n",BOARD_USER_MIPS_GPIO_5_PURPOSE_TEXT);
	PRINTF("\t\t%s\r\n",BOARD_USER_MIPS_GPIO_6_PURPOSE_TEXT);
#else
	PRINTF("\tNo MIPS measure by GPIO\r\n\n");
#endif

    return kStatus_SHELL_Success;
}

