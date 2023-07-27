/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// general
#include "appGlobal.h"

// board
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

// PDM
#include "fsl_pdm.h"

//INIT
#include "init.h"

#if (APP_PLATFORM == APP_PL_RT1170EVK)

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*****************************************************************************************
 * Function
 *****************************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
/*
 * AUDIO PLL setting: Frequency = Fref * (DIV_SELECT + NUM / DENOM) / (2^POST)
 *                              = 24 * (32 + 77/100)  / 2
 *                              = 393.24MHZ
 */

const clock_audio_pll_config_t s_audioPllConfig = {
												.loopDivider = 32,  /* PLL loop divider. Valid range for DIV_SELECT divider value: 27~54. */
												.postDivider = 1,   /* Divider after the PLL, should only be 0, 1, 2, 3, 4, 5 register POST_DIV_SEL */
												.numerator   = 77,  /* 30 bit numerator of fractional loop divider. */
												.denominator = 100, /* 30 bit denominator of fractional loop divider */
												};

/*
 * PDM
 * Default configuration but will be updated according audio Tx path configuration, see Tx command shell
 */
// PDM IP general configuration
pdm_config_t g_pdmConfig         = {
											.enableDoze        = false,
											.fifoWatermark     = APP_AUDIO_PDM_FIFO_WATERMARK,
											.qualityMode       = APP_AUDIO_PDM_QUALITY_MODE,
											.cicOverSampleRate = APP_AUDIO_PDM_CIC_OVERSAMPLE_RATE,
											};
// PDM each channel
pdm_channel_config_t g_pdmChannelConfig = {
											.cutOffFreq = kPDM_DcRemoverCutOff152Hz,
											.gain       = kPDM_DfOutputGain2,    		// gain 0dB with high quality by default
											};

/*******************************************************************************
 * Code
 ******************************************************************************/

/************************************************************************************
initBoardClock
************************************************************************************/

status_t initBoardClock(void)
{
    status_t retStatus     = kStatus_Success;
    status_t retStatusFunc = kStatus_Success;

    //GPIO for Led configuration
    gpio_pin_config_t gpioLedConfig 		= {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};

    //GPIO for Led configuration
    gpio_pin_config_t gpioUserButtonConfig 	= {kGPIO_DigitalInput, 0, kGPIO_NoIntmode};

    // GPIO for MIPS measure on J26
#ifdef MIPS_MEASURE_GPIO
    gpio_pin_config_t gpioMipsConfig 		= {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};
#endif

	// Config for mainBoard MIMRT1170EVK_REVA
	BOARD_ConfigMPU();
	BOARD_InitPins();
	BOARD_BootClockRUN();

	CLOCK_InitAudioPll(&s_audioPllConfig);
	BOARD_InitDebugConsole();

	// Init output LED controlled by GPIO
	GPIO_PinInit( BOARD_USER_LED_GPIO, BOARD_USER_LED_GPIO_PIN,   	&gpioLedConfig);
	GPIO_PinInit( BOARD_USER_LED_GPIO, BOARD_USER_LED_GPIO_PIN_2,	&gpioLedConfig);

	// Init input SW7 controller by GPIO
	GPIO_PinInit( BOARD_USER_BUTTON_GPIO, BOARD_USER_BUTTON_GPIO_PIN, &gpioUserButtonConfig);

	// Init GPIO as output for Mips measure
#ifdef MIPS_MEASURE_GPIO
	GPIO_PinInit( BOARD_USER_MIPS_GPIO_1, BOARD_USER_MIPS_GPIO_1_PIN, &gpioMipsConfig);
	GPIO_PinInit( BOARD_USER_MIPS_GPIO_2, BOARD_USER_MIPS_GPIO_2_PIN, &gpioMipsConfig);
	GPIO_PinInit( BOARD_USER_MIPS_GPIO_3, BOARD_USER_MIPS_GPIO_3_PIN, &gpioMipsConfig);
	GPIO_PinInit( BOARD_USER_MIPS_GPIO_4, BOARD_USER_MIPS_GPIO_4_PIN, &gpioMipsConfig);
	GPIO_PinInit( BOARD_USER_MIPS_GPIO_5, BOARD_USER_MIPS_GPIO_5_PIN, &gpioMipsConfig);
	GPIO_PinInit( BOARD_USER_MIPS_GPIO_6, BOARD_USER_MIPS_GPIO_6_PIN, &gpioMipsConfig);
#endif

	return kStatus_Success;
}
/************************************************************************************
initBoarClockEnableSaiMclkOutput
************************************************************************************/
status_t initBoarClockEnableSaiMclkOutput(bool enable)
{
    if (enable)
    {
        IOMUXC_GPR->GPR0 |= IOMUXC_GPR_GPR0_SAI1_MCLK_DIR_MASK;
    }
    else
    {
        IOMUXC_GPR->GPR0 &= (~IOMUXC_GPR_GPR0_SAI1_MCLK_DIR_MASK);
    }
    return kStatus_Success;
}

#endif
