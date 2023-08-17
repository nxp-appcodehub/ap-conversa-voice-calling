/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// general
#include "appGlobal.h"
#include "init.h"

// board
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

//Flexcom
#include "fsl_sysctl.h"

#if (APP_PLATFORM == APP_PL_LPC55S69EVK)

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
/*******************************************************************************
 * Code
 ******************************************************************************/
//*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            initBoardClock										 					 */
/*                                                                                               */
/* DESCRIPTION: Init required board clock                                  		 				 */
/*                                                                                               */
/* PARAMETERS:                                                                                   */
/*************************************************************************************************/
status_t initBoardClock(void)
{
	status_t retStatus     = kStatus_Success;
	status_t retStatusFunc = kStatus_Success;
	gpio_pin_config_t led_config =
	{
		kGPIO_DigitalOutput, 0
	};
	/* USART0 clock */
	CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

	BOARD_InitBootPins();
	BOARD_BootClockPLL150M();
	BOARD_InitDebugConsole();				//fsl_debug_console.c and fsl_debug_console.h are copied from SDK shell example, with some modifications, search for "stan"

	CLOCK_EnableClock(kCLOCK_InputMux);
	CLOCK_EnableClock(kCLOCK_Iocon);
	/* I2C clock */
	CLOCK_AttachClk(kFRO12M_to_FLEXCOMM4);

	/* reset FLEXCOMM for I2C */
	RESET_PeripheralReset(kFC4_RST_SHIFT_RSTn);

	/* reset FLEXCOMM for DMA0 */
	RESET_PeripheralReset(kDMA0_RST_SHIFT_RSTn);

	/* Attach PLL clock to MCLK for I2S, no divider */
	CLOCK_AttachClk(kPLL0_to_MCLK);
	SYSCON->MCLKDIV = SYSCON_MCLKDIV_DIV(0U);
	SYSCON->MCLKIO  = 1U;

	//measured result: enabe this, can speedup 2% (code running in flash, PDM converting 2ch, debug, 0.43ms --> 0.42ms )
	#if !defined(DONT_ENABLE_FLASH_PREFETCH)
		//SYSCON->FMCCR |= SYSCON_FMCCR_PREFEN_MASK;
	#endif
#ifdef MIPS_MEASURE_GPIO
	CLOCK_EnableClock(kCLOCK_Gpio0);
	CLOCK_EnableClock(kCLOCK_Gpio1);
	GPIO_PinInit (GPIO, 1,BOARD_USER_MIPS_GPIO_0_PIN, &led_config);
	GPIO_PinInit (GPIO, 1,BOARD_USER_MIPS_GPIO_1_PIN, &led_config);
	GPIO_PinInit (GPIO, 1,BOARD_USER_MIPS_GPIO_3_PIN, &led_config);
	GPIO_PinInit (GPIO, 1,BOARD_USER_MIPS_GPIO_4_PIN, &led_config);

#endif
	return kStatus_Success;
}
/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            initFlexcommPinsSharing										 			 */
/*                                                                                               */
/* DESCRIPTION: declare I2S pin sharing                                		 				     */
/*                                                                                               */
/* PARAMETERS:                                                                                   */
/*************************************************************************************************/
status_t initFlexcommPinsSharing(AUDIO_definition_st* 	 p_definition)
{
	if (p_definition == NULL)
	{
		return kStatus_NullPointer;
	}
    SYSCTL_Init(SYSCTL);
    /* select signal source for share set */
    SYSCTL_SetShareSignalSrc(SYSCTL, kSYSCTL_ShareSet0, kSYSCTL_SharedCtrlSignalSCK, kSYSCTL_Flexcomm7);
    SYSCTL_SetShareSignalSrc(SYSCTL, kSYSCTL_ShareSet0, kSYSCTL_SharedCtrlSignalWS, kSYSCTL_Flexcomm7);

    SYSCTL_SetShareSignalSrc(SYSCTL, kSYSCTL_ShareSet1, kSYSCTL_SharedCtrlSignalSCK, kSYSCTL_Flexcomm2);
    SYSCTL_SetShareSignalSrc(SYSCTL, kSYSCTL_ShareSet1, kSYSCTL_SharedCtrlSignalWS, kSYSCTL_Flexcomm2);
    /* select share set for flexcomm2,6,7 */
    SYSCTL_SetShareSet(SYSCTL, kSYSCTL_Flexcomm7, kSYSCTL_FlexcommSignalSCK, kSYSCTL_ShareSet0);
    SYSCTL_SetShareSet(SYSCTL, kSYSCTL_Flexcomm7, kSYSCTL_FlexcommSignalWS, kSYSCTL_ShareSet0);

    SYSCTL_SetShareSet(SYSCTL, kSYSCTL_Flexcomm2, kSYSCTL_FlexcommSignalSCK, kSYSCTL_ShareSet1);
    SYSCTL_SetShareSet(SYSCTL, kSYSCTL_Flexcomm2, kSYSCTL_FlexcommSignalWS, kSYSCTL_ShareSet1);
}

#endif
