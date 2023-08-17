/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __TUNING_CONF_H__
#define __TUNING_CONF_H__

#include "stdint.h"



/*******************************************************************************
 * Structure
 ******************************************************************************/
typedef struct
{
	uint32_t settings_format_version;
	uint32_t conversa_lib_version[3];
	uint32_t num_mic;
	uint32_t num_spk;
	uint32_t tx_samplerate;
	uint32_t rx_samplerate;
	uint32_t dsp_blocksize;
	uint32_t num_bands;
	char     info_str[128];   				// string information
	uint32_t parameter_data_size;
	uint32_t parameter_data[];				// parameters table
} conversa_parameter_config_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/
/* SOFTWARE PACK CONFIGURATION */
extern const conversa_parameter_config_t conversa_parameter_config_LPC55S69swp16k;

/*end SOFTWARE PACK CONFIGURATION */

/* CUSTOMER CONFIGURATION */
// add here new conversa configuration
/*end CUSTOMER CONFIGURATION */

#endif/*__TUNING_CONF_H__*/
