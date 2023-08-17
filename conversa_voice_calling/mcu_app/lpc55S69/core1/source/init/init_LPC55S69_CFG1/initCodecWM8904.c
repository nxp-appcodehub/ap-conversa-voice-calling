/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

//Standard headers
#include <init.h>
#include <stdint.h>

//Platform headers
#include "board.h"

//fsl driver headers
#include "fsl_i2c.h"
#include "fsl_wm8904.h"
#include "fsl_codec_adapter.h"
#include "fsl_codec_common.h"

//Application headers
#include "appGlobal.h"


#if (APP_PLATFORM == APP_PL_LPC55S69EVK)

#if RX_PATH_PRESENT
/*******************************************************************************
 * Macro Definitions
 ******************************************************************************/


/*******************************************************************************
 * Prototypes
 ******************************************************************************/


/*******************************************************************************
 * Variables
 ******************************************************************************/

codec_handle_t codecHandle;
wm8904_config_t wm8904Config = {
    .i2cConfig    = {.codecI2CInstance = BOARD_CODEC_I2C_INSTANCE, .codecI2CSourceClock = BOARD_CODEC_I2C_CLOCK_FREQ},
    .recordSource = kWM8904_RecordSourceLineInput,
    .recordChannelLeft  = kWM8904_RecordChannelLeft2,
    .recordChannelRight = kWM8904_RecordChannelRight2,
    .playSource         = kWM8904_PlaySourceDAC,
    .slaveAddress       = WM8904_I2C_ADDRESS,
    .protocol           = kWM8904_ProtocolI2S,
    .format             = {.sampleRate = APP_AUDIO_CODEC_SAMPLERATE, .bitWidth = APP_AUDIO_CODEC_BITWIDTH},
    .mclk_HZ            = APP_AUDIO_I2S_CLK_FREQ,
    .master             = false,
};
codec_config_t boardCodecConfig = {.codecDevType = kCODEC_WM8904, .codecDevConfig = &wm8904Config};


/*******************************************************************************
 * Code
 ******************************************************************************/
/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            initCodecWM8904										 			 		 */
/*                                                                                               */
/* DESCRIPTION: init Codec WM8904 + set volume                                		 		     */
/*                                                                                               */
/* PARAMETERS:                                                                                   */
/*************************************************************************************************/
void initCodecWM8904(void)
{
	PL_INT32 Vol=75;
    if (CODEC_Init(&codecHandle, &boardCodecConfig) != kStatus_Success)
    {
        assert(false);
    }
    if (CODEC_SetVolume(&codecHandle, kCODEC_PlayChannelHeadphoneLeft | kCODEC_PlayChannelHeadphoneRight, Vol) !=
        kStatus_Success)
    {
        assert(false);
    }
}

void BOARD_SetCodecMuteUnmute(bool mute)
{
    if (CODEC_SetMute(&codecHandle, kCODEC_PlayChannelHeadphoneLeft | kCODEC_PlayChannelHeadphoneRight, mute) !=
        kStatus_Success)
    {
        assert(false);
    }
}

#endif
#endif /*#if (APP_PLATFORM == APP_PL_LPC55S69EVK)*/

