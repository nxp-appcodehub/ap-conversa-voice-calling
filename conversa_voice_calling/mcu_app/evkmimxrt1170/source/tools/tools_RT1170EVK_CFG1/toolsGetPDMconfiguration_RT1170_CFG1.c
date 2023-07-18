/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "tools.h"

#include "appGlobal.h"

#if (APP_PLATFORM == APP_PL_RT1170EVK)

/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            toolsGetPdmConfiguration												 */
/*                                                                                               */
/* DESCRIPTION:         return the pdm configuration according the audio configuration path      */
/*                                                                                               */
/* PARAMETERS:                                                                                   */
/*************************************************************************************************/
status_t toolsGetPdmConfiguration (  pdm_config_t* 		   p_pdmConfig,
		 						   	 pdm_channel_config_t* p_pdmChannelConfig,
									 AUDIO_configParam_st* p_configParam)
{
	status_t retStatus = kStatus_Success;

    return retStatus;
}

#endif
