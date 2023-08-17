/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */



//Application headers
#include "appGlobal.h"
#include "tools.h"
#include "fsl_crc.h"

void ResetCrc32Seed(void)
{
	CRC_ENGINE->SEED = 0xffffffff;
}

void Init_CrcEngine(void)
{
    crc_config_t config;
    config.polynomial    = kCRC_Polynomial_CRC_32;
    config.reverseIn     = false;
    config.complementIn  = false;
    config.reverseOut    = false;
    config.complementOut = false;	//note --- these true false settings are all set to false, different from the driver demo program
    config.seed          = 0xFFFFFFFFU;
    CRC_Init(CRC_ENGINE, &config);
}

