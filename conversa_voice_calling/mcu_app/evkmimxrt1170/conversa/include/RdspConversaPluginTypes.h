/*
 * Copyright 2021 by Retune DSP
 * Copyright 2022 NXP
 * 
 * SPDX-License-Identifier: BSD-3-Clause
*/


#ifndef RDSP_CONVERSA_PLUGIN_TYPES_H_
#define RDSP_CONVERSA_PLUGIN_TYPES_H_

#ifdef _WIN32
//#include <cstdint>
#else
#include <stdint.h>
#endif

#include "RdspConversaPlatforms.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	RESAMPLING_NONE = 0, RESAMPLE_FACTOR_TWO = 2, RESAMPLE_FACTOR_THREE = 3, RESAMPLE_FACTOR_TWO_THIRDS = 4
} RDSP_RESAMPLER_MODES;


//	rdsp types				RDSP_USES_IEEE754_CODE			HIFI4/FUSIONF1
//-------------------------------------------------------------------------------
// rdsp_float				float							xtfloat
// rdsp_power				float							xtfloat
// rdsp_dB:					float							xtfloat
// rdsp_floatx2:			float[2]						xtfloatx2
// rdsp_complex:			float[2]						xtfloatx2
// rdsp_double:				double							xtfloatx2
//-------------------------------------------------------------------------------

#if defined(HIFI4) || defined(FUSIONDSP)

typedef xtfloat rdsp_float;
typedef xtfloat rdsp_dB;
typedef xtfloatx2 rdsp_floatx2;
typedef xtfloatx2 rdsp_complex;
typedef xtfloatx2 rdsp_double;

#else

typedef float rdsp_float;
typedef float rdsp_dB;
typedef float rdsp_floatx2[2];
typedef float rdsp_complex[2];
typedef double rdsp_double;

#endif

typedef struct rdsp_model_ver_struct {
	int32_t major;
	int32_t minor;
	int32_t patch;
} rdsp_model_version;

typedef enum {
	NO_TEST = -1,
	TESTMIC0 = 0,
	TESTMIC1 = 1,
	TESTMIC2 = 2,
	TESTMIC3 = 3,
	TESTMIC4 = 4,
	TESTMIC5 = 5,
	TESTMIC6 = 6,
	TESTMIC7 = 7,
	SUMALLMICS = 8,
	RXOUT_CH0_TO_TXOUT = 9,
	RXOUT_CH1_TO_TXOUT = 10,
	RXIN_CH0_TO_TX_OUT = 11,
	RXIN_CH1_TO_TX_OUT = 12
} RDSP_MIC_TEST_MODE;

typedef enum {
	NearTalk, FarTalk, NoTalk, DoubleTalk
} DOA_Talker_Tracking;

#ifdef __cplusplus
}
#endif

#endif // RDSP_CONVERSA_PLUGIN_TYPES_H_
