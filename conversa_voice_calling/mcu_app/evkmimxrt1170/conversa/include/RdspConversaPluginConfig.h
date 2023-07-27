/*
 * Copyright 2021 by Retune DSP
 * Copyright 2022 NXP
 * 
 * SPDX-License-Identifier: BSD-3-Clause
*/


#ifndef RDSP_CONVERSA_PLUGIN_CONFIG_H_
#define RDSP_CONVERSA_PLUGIN_CONFIG_H_

#include "RdspDeviceConfig.h"
#include "RdspConversaPluginTypes.h"

#ifdef __cplusplus
extern "C" {
#endif
	
typedef struct rdsp_conversa_plugin_config_s {
	uint32_t aec_uses_current_sensing;
	uint32_t create_doa;
	uint32_t num_bf;
	uint32_t bf_num_mics;
	uint32_t num_aec;
	uint32_t aec_num_mics;
	uint32_t aec_num_refs;
	int32_t se_model_blob_is_already_open;
	RDSP_DeviceId_en device_id;
} rdsp_conversa_plugin_config_t;

#ifdef __cplusplus
}
#endif

#endif // RDSP_CONVERSA_PLUGIN_CONFIG_H_
