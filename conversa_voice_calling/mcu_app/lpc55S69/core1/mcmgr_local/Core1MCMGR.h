/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CORE1_MCMGR_H
#define CORE1_MCMGR_H

#define CLEAR_VALUE 	0xFFFFFFFF

status_t CORE1_initMCMGR(void);
status_t CORE1_sendSharedStructAddress(PL_UINT32 SharedStructAddress);
#endif /*CORE1_MCMGR_H*/




