/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __IT_CB_H__
#define __IT_CB_H__

// freeRtos
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

// pdm edma
#include "fsl_pdm_edma.h"

// sai edma
#include "fsl_sai_edma.h"


/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Structure
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/


/*****************************************************************************************
 * Function
 *****************************************************************************************/
/*
 * callBack_pdmEdma
 *     call back function called by the edma reserved for PDM when a transfer is done
 */
void callBack_edmaPdmMics(PDM_Type *base, pdm_edma_handle_t *handle, status_t status, void *userData);

/*
 * callBack_saiTx
 *     call back function called by the edma reserved for SAI Tx when a transfer is done
 */
void callBack_saiTx(I2S_Type *base, sai_edma_handle_t *handle, status_t status, void *userData);

#endif /* __IT_CB_H__ */



