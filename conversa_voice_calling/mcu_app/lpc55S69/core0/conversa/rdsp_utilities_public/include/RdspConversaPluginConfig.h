/*
* Copyright 2021 by Retune DSP
* Copyright 2022 NXP
*
* NXP Confidential and Propietary. This software is owned or controlled by NXP 
* and may only be used strictly in accordance with the applicable license terms.  
* By expressly accepting such terms or by downloading, installing, 
* activating and/or otherwise using the software, you are agreeing that you have read, 
* and that you agree to comply with and are bound by, such license terms.  
* If you do not agree to be bound by the applicable license terms, 
* then you may not retain, install, activate or otherwise use the software.
*
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
