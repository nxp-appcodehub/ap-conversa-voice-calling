/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __Conversa_h__
#define __Conversa_h__

#include"fsl_common.h"
// CMSIS lib
#include "arm_math.h"
#define LPC_CONTROL_ADDR 0x2003FFF0
/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            initSetConversa				   											 */
/*                                                                                               */
/* DESCRIPTION: set conversa configuration								  	 					 */
/*              create conversa instance 														 */
/*                                                                                               */
/* PARAMETERS:                                                                                   */
/*************************************************************************************************/
extern status_t initSetConversa(void *PtrCommonVarSharedByCore0AndCore1);
/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            swProcessConversa			   										 	 */
/*                                                                                               */
/* DESCRIPTION: process input audio data by conversa 			  	 					 		 */
/*                                                                                               */
/* PARAMETERS:                                                                                   */
/*         Shared Structure address                              	 							 */
/*************************************************************************************************/
extern status_t swProcessConversa(void *PtrCommonVarSharedByCore0AndCore1);

extern void arm_float_to_q31(const float32_t * pSrc,q31_t * pDst, uint32_t blockSize);
extern void arm_q31_to_float(const q31_t * pSrc,float32_t * pDst, uint32_t blockSize);
#endif

