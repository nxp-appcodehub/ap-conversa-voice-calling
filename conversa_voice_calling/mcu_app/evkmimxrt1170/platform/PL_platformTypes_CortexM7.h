/*
 * Copyright 2020-2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#ifndef _PLATFORM_CORTEXM7_H_
#define _PLATFORM_CORTEXM7_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef     signed char            PL_INT8;  
typedef     short                  PL_INT16; 
typedef     int                    PL_INT32; 
typedef     long long              PL_INT64; 
typedef     unsigned char          PL_UINT8; 
typedef     unsigned short         PL_UINT16;
typedef     unsigned int           PL_UINT32;
typedef     unsigned long long     PL_UINT64;
typedef     float                  PL_FLOAT; 
typedef     double                 PL_DOUBLE;
typedef     unsigned short         PL_BOOL;
typedef     PL_UINT32              PL_UINTPTR;

#define     PL_NULL                NULL                ///< NULL pointer
#define 	PL_INT32_MIN		   -2147483648
#define 	PL_INT32_MAX		   2147483647
#define     PL_MAXENUM             PL_INT32_MAX        ///< Maximum value for enumerator


// PL_BOOL
enum { PL_FALSE, PL_TRUE};

// Memory alignment
#define     PL_MEM_ALIGN(var, alignbytes)      var __attribute__((aligned(alignbytes)))

#include "PL_memoryRegion.h"

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*_PLATFORM_CORTEXM7_H_*/
