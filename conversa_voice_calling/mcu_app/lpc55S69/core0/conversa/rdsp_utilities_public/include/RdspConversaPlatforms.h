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

#ifndef RDSP_CONVERSA_PLATFORMS_H
#define RDSP_CONVERSA_PLATFORMS_H

#include <stdint.h>

#if RDSP_LIB_USES_POWERQUAD
#include "fsl_powerquad.h"
#endif

#if defined(FUSIONDSP)
#include <xtensa/tie/xt_fusion.h>
#endif
#if defined(HIFI3)
#include <xtensa/tie/xt_hifi3.h>
#endif
#if defined(HIFI4)
#include <xtensa/tie/xt_hifi4.h>
#endif

/* NatureDSP lib */
#if defined(HIFI3) || defined(HIFI4) || defined(FUSIONDSP)
#include "NatureDSP_Signal.h"
#include "NatureDSP_types.h"
#endif

#if defined(HMD1A)
#include "Biquad.h"
#include "fr32_math.h"
#include "fr32_utils.h"
#include "fr32_xcc_ops.h"
#include "TIE_defs.h"
#include "TIE_include.h"
#include "TIE_DSPInternal.h"
#include "CoreLib.h"
#include "FFT.h"
#endif

#if defined(LITTLE_KERNEL)
#define RDSP_CONVERSA_DISABLE_FILEIO (1)
#define __ALWAYS_INLINE __attribute__((always_inline))
#endif

#ifdef _WIN32
#define RDSP_PRAGMA_USED 
#else
#define RDSP_PRAGMA_USED __attribute__((used))
#endif

#ifdef __ARM_NEON
#define RDSP_PACKED __attribute__((__packed__))
#else
#define RDSP_PACKED
#endif

#ifdef __LPC_CONFIG
#define RDSP_PUT_FUNCTION_IN_FLASH      __attribute__((__section__("CreateCodeInFlash")))		   // will be placed in Flash
#define RDSP_PUT_FUNCTION_IN_DATA_RAM    __attribute__((__section__("CodeQuickAccessInDataRam")))  // will be placed in Data RAM
#define RDSP_PUT_FUNCTION_IN_CODE_RAM    __attribute__((__section__("CodeQuickAccessInCodeRam")))  // will be placed in Code RAM
#else
#define RDSP_PUT_FUNCTION_IN_FLASH
#define RDSP_PUT_FUNCTION_IN_DATA_RAM
#define RDSP_PUT_FUNCTION_IN_CODE_RAM
#endif

#endif // RDSP_CONVERSA_PLATFORMS_H
