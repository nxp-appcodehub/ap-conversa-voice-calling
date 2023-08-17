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


#ifndef RDSP_CONVERSA_PLUGIN_H_
#define RDSP_CONVERSA_PLUGIN_H_

#include "RdspConversaPluginConfig.h"
#include "RdspConversaStatusCodes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct rdsp_conversa_plugin_s {
	uint8_t id; /*!< Plugin instance ID   */
	uint8_t state; /*!< Plugin running state */
	uint8_t num_mics;
	uint8_t num_rx_in;
	uint8_t num_spks;
	void* extmem_base_address;
	uint32_t ParamBlockSize_bytes;
	uint32_t extmem_size_bytes;
	uint32_t  paramBlockState;
	uint32_t *pParamBlock;
	uint8_t* SE_model_ptr;
	uint32_t SE_model_size;
	rdsp_conversa_plugin_config_t config;
} rdsp_conversa_plugin_t;

typedef struct rdsp_conversa_plugin_constants_s {
	uint32_t rx_fs;
	uint32_t tx_fs;
	uint32_t audio_framesize;
	uint32_t max_num_mics;
	uint32_t max_num_spks;
	uint32_t num_bytes_per_sample;
	uint32_t build_for_headset;
	uint32_t build_for_speakerphone;
} rdsp_conversa_plugin_constants_t;

extern void RdspConversa_Plugin_GetConstants(rdsp_conversa_plugin_constants_t* Aconversa_constants);
extern RdspStatus RdspConversa_Plugin_Create(rdsp_conversa_plugin_t *APluginInit_s);
extern RdspStatus RdspConversa_Plugin_Init(rdsp_conversa_plugin_t *APluginInit_s);

extern RdspStatus RdspConversa_Plugin_Process_Q31(rdsp_conversa_plugin_t *APluginInit_s, int32_t** Amic_in, int32_t** Arx_in, int32_t** Aopt_current_ref /*pass anything when not using current sensing*/);
extern int32_t** RdspConversa_Plugin_GetRxOut_Q31(rdsp_conversa_plugin_t *APluginInit_s);
extern int32_t* RdspConversa_Plugin_GetTxOut_Q31(rdsp_conversa_plugin_t *APluginInit_s);
extern int32_t* RdspConversa_Plugin_GetTxBfOut_Q31(rdsp_conversa_plugin_t *APluginInit_s);

extern RdspStatus RdspConversa_Plugin_Process(rdsp_conversa_plugin_t *APluginInit_s, rdsp_float ** Amic_in, rdsp_float** Arx_in, rdsp_float** Aopt_current_ref /*pass anything when not using current sensing*/);
extern rdsp_float** RdspConversa_Plugin_GetRxOut(rdsp_conversa_plugin_t *APluginInit_s);
extern rdsp_float* RdspConversa_Plugin_GetTxOut(rdsp_conversa_plugin_t *APluginInit_s);
extern rdsp_float* RdspConversa_Plugin_GetTxBfOut(rdsp_conversa_plugin_t *APluginInit_s);

extern void RdspConversa_Plugin_GetLibVersion(rdsp_conversa_plugin_t *APluginInit_s, uint32_t* Amajor, uint32_t* Aminor, uint32_t* Apatch);
extern uint32_t RdspConversa_Plugin_GetAllocatedMemoryBytes(rdsp_conversa_plugin_t *APluginInit_s);
extern uint32_t RdspConversa_Plugin_GetRequiredHeapMemoryBytes(rdsp_conversa_plugin_t* APluginInit_s);

extern void* RdspConversa_Plugin_GetControlDataAddress(rdsp_conversa_plugin_t *APluginInit_s);
extern void RdspConversa_Plugin_GetSignal(rdsp_conversa_plugin_t *APluginInit_s, uint32_t Aid, uint32_t Alength, uint32_t* Abuffer) ;
extern void RdspConversa_Plugin_SetParameters(rdsp_conversa_plugin_t *APluginInit_s, const char* Ainfo, int32_t Ainfo_length, const uint32_t* Aparameters, uint32_t Anum_par_words);
extern void RdspConversa_Plugin_SetParametersUsingBinData(rdsp_conversa_plugin_t *APluginInit_s, const uint32_t* Aparameters, uint32_t Anum32bWords);
extern int32_t RdspConversa_Plugin_SetSingleParameter(rdsp_conversa_plugin_t *APluginInit_s, uint32_t* Aparameter_data) ;
extern void RdspConversa_Plugin_SetAecCoefs(rdsp_conversa_plugin_t *APluginInit_s, char* Ainfo_aec, int32_t Ainfo_aec_length, uint32_t* Aparameters);
extern void RdspConversa_Plugin_SetVolume(rdsp_conversa_plugin_t *APluginInit_s, int32_t Avolume_level);
extern uint64_t RdspConversa_Plugin_GetFrameCounter(rdsp_conversa_plugin_t *APluginInit_s);
extern int32_t RdspConversa_Plugin_GetTargetDirection(rdsp_conversa_plugin_t *APluginInit_s);
extern int32_t RdspConversa_Plugin_GetDoaOperationalState(rdsp_conversa_plugin_t *APluginInit_s, DOA_Talker_Tracking* ADoaOpState);
extern int32_t RdspConversa_Plugin_GetSpeechQualityIndicatorState(rdsp_conversa_plugin_t *APluginInit_s);

extern void RdspConversa_Plugin_EnableBeamformer(rdsp_conversa_plugin_t *APluginInit_s, int32_t Aenable_flag);

extern void RdspConversa_Plugin_MicResample_Create(rdsp_conversa_plugin_t *APluginInit_s, int32_t AresampleFactor, int32_t Anhop_decim_in /* derive the nhop_decim from Conversa NHOP and resample mode if NULL*/, int32_t Anhop_interp_in /* derive the nhop_interp from Conversa NHOP and resample mode if NULL*/);
extern void RdspConversa_Plugin_MicResample_Downsample_Process(rdsp_float** Amic_in, int32_t Anum_channels);
extern void RdspConversa_Plugin_MicResample_Upsample_Process(rdsp_float** Amic_in, int32_t Anum_channels);
extern void RdspConversa_Plugin_MicResample_Destroy(rdsp_conversa_plugin_t *APluginInit_s);

extern void RdspConversa_Plugin_SpkResample_Create(rdsp_conversa_plugin_t *APluginInit_s, int32_t AresampleFactor, int32_t Anhop_decim_in /* derive the nhop_decim from Conversa NHOP and resample mode if NULL */, int32_t Anhop_interp_in /* derive the nhop_interp from Conversa NHOP and resample mode if NULL*/);
extern void RdspConversa_Plugin_SpkResample_Downsample_Process(rdsp_float** Aspkc_in, int32_t Anum_channels);
extern void RdspConversa_Plugin_SpkResample_Upsample_Process(rdsp_float** Aspk_in, int32_t Anum_channels);
extern void RdspConversa_Plugin_SpkResample_Destroy(rdsp_conversa_plugin_t *APluginInit_s);

#if CONVERSA_PRINT_PROFILING
extern void RdspConversa_Plugin_PrintResourceOverview();
extern void RdspConversa_Plugin_PrintMaxMipsUsageOverview();
#endif

extern void RdspConversa_Plugin_Destroy(rdsp_conversa_plugin_t *APluginInit_s);

#ifdef __cplusplus
}
#endif

#endif // RDSP_CONVERSA_PLUGIN_H_
