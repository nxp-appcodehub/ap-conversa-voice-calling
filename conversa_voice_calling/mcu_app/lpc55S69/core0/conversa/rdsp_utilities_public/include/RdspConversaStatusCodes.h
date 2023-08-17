/*
* Copyright 2021 by Retune DSP
* Copyright 2022 NXP
*
* NXP Confidential and Propietary. This software is owned or controlled by NXP 
* and may only be used strictly in accordance with the applicable license terms.  
* By expressly accepting such terms or by downloading, installing, 
* activating and/or otherwise using the software, you are agreeing that you have read, 
* and that you agree to comply with and are bound by, such license terms.  
* If you do not agree to be bound by the applicable license terms, 
* then you may not retain, install, activate or otherwise use the software.
*
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


