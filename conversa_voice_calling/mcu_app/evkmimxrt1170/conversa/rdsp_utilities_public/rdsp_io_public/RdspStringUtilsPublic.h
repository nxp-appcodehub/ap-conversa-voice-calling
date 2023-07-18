/*
* Copyright 2021 Retune DSP 
* Copyright 2022 NXP 
*
* SPDX-License-Identifier: BSD-3-Clause
*
*/
#ifndef RDSP_STRING_UTILS_PUBLIC
#define RDSP_STRING_UTILS_PUBLIC

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif


#if SDK_DEBUGCONSOLE
// Use FSL driver for MCUXpresso builds
extern int DbgConsole_Printf(const char *formatString, ...);
#define rdsp_printf(...) DbgConsole_Printf(__VA_ARGS__)
#else
// Use stdio
#define rdsp_printf(...) printf(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif /* RDSP_STRING_UTILS_PUBLIC */


