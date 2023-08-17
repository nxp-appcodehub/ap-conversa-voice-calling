/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#ifndef __IRQ_CallBACK_PDMANDSAI___
#define __IRQ_CallBACK_PDMANDSAI___
#if RX_PATH_PRESENT
extern void callBack_I2S_DMA_Rx(I2S_Type 		 *base,
								i2s_dma_handle_t *handle,
								status_t 		 completionStatus,
								void 			 *userData);
#endif
extern void callBack_I2S_DMA_Tx(I2S_Type 		 *base,
								i2s_dma_handle_t *handle,
								status_t 		 completionStatus,
								void 			 *userData);
extern void MAILBOX_IRQHandler(void);
#endif
