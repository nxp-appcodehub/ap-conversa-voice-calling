/*
* Copyright 2021 Retune DSP 
* Copyright 2022 NXP 
*
* SPDX-License-Identifier: BSD-3-Clause
*
*/
#ifndef RDSP_FILE_UTILS_PUBLIC
#define RDSP_FILE_UTILS_PUBLIC

//#if RDSP_CONVERSA_LIB_ENABLE_FILEIO

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#if USE_FATFS
#include "ff.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	FILE_OK = 0,
    FILE_GENERAL_ERROR = 1,
} RdspFileStatus;

extern RdspFileStatus rdsp__fopen(void* Afile, char* Afilename, const char* Arw);
extern RdspFileStatus rdsp__fwrite(void* Ain, int32_t Asize, int32_t Anum, void* Afile);

#ifdef __cplusplus
}
#endif

//#endif // RDSP_CONVERSA_LIB_ENABLE_FILEIO

#endif /* RDSP_MEMORY_UTILS_PUBLIC */


