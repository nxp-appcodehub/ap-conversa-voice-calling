/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CORE0_MCMGR_H
#define CORE0_MCMGR_H

#define CLEAR_VALUE 	0xFFFFFFFF

/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            CORE0_checkSharedStrucrAddress							 				 */
/*                                                                                               */
/* DESCRIPTION:    	   Ensure that sharedStruct address has been well received                   */
/* PARAMETERS:                                        											 */
/*                                                                                               */
/*************************************************************************************************/
PL_BOOL CORE0_checkSharedStrucrAddress(void);
/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            CORE0_initMCMGR							 								 */
/*                                                                                               */
/* DESCRIPTION:    	    Init MCMGR on Core0 + assigned user event                                */
/* PARAMETERS:                                        											 */
/*                                                                                               */
/*************************************************************************************************/
status_t CORE0_initMCMGR(void);

#endif /*CORE0_MCMGR_H*/




