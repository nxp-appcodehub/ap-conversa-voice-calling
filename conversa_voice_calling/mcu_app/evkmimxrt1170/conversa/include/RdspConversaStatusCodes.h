/*
 * Copyright 2021 by Retune DSP
 * Copyright 2022 NXP
 * 
 * SPDX-License-Identifier: BSD-3-Clause
*/


#ifndef RDSP_CONVERSA_STATUS_CODES_H_
#define RDSP_CONVERSA_STATUS_CODES_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    OK = 0,
    GENERAL_ERROR = 1,
    MALLOC_FAIL = 2,
    INVALID_PARAMETERS = 3,
    LICENSE_EXPIRED = 4
} RdspStatus;


#ifdef __cplusplus
}
#endif

#endif // RDSP_CONVERSA_STATUS_CODES_H_


