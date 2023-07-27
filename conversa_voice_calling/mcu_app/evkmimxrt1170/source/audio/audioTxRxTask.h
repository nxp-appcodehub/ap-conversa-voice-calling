/*
 * Copyright 2020-2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __AUDIO_TXRX_TASK_H__
#define __AUDIO_TXRX_TASK_H__

/* AUDIO TX is consider as local source to network path */

#include "audio.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define AUDIO_TASK_STACK_SIZE       (4*1024)
#define AUDIO_TASK_PRIORITY         configMAX_PRIORITIES-1 		// configMAX_PRIORITIES-1 is the most priority task and idle task priority is 0

/*******************************************************************************
 * Structure
 ******************************************************************************/

/*******************************************************************************
 * Enum
 ******************************************************************************/

/*******************************************************************************
 * Fonction
 ******************************************************************************/
void AUDIO_TxRxTask(void* param);

#endif /* __AUDIO_TXRX_TASK_H__ */
