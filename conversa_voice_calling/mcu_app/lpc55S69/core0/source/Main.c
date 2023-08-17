/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


// board & pin
#include "pin_mux.h"
#include "board.h"
// fsl drivers
#include "fsl_debug_console.h"
#include "fsl_gpio.h"
#include "fsl_power.h"
#include "fsl_mailbox.h"
#include "fsl_powerquad.h"
// others
#include "PL_platformTypes.h"
#include "Core0MCMGR.h"
#include "mcmgr.h"



/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Address of RAM, where the image for core1 should be copied */
#define CORE1_BOOT_ADDRESS 0x80000
#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
extern uint32_t Image$$CORE1_REGION$$Base;
extern uint32_t Image$$CORE1_REGION$$Length;
#define CORE1_IMAGE_START &Image$$CORE1_REGION$$Base
#elif defined(__ICCARM__)
extern unsigned char core1_image_start[];
#define CORE1_IMAGE_START core1_image_start
#elif (defined(__GNUC__)) && (!defined(__MCUXPRESSO))
extern const char core1_image_start[];
extern const char *core1_image_end;
extern uint32_t core1_image_size;
#define CORE1_IMAGE_START ((void *)core1_image_start)
#define CORE1_IMAGE_SIZE  ((void *)core1_image_size)
#endif

#define GET_CYCLE_COUNTER_Z(x)   x=DWT->CYCCNT;
volatile PL_UINT32 CycleCounter1=0;
volatile PL_UINT32 CycleCounter2=0;
/*******************************************************************************
 * Prototypes
 ******************************************************************************/



/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Application-specific implementation of the SystemInitHook() weak function.
 */
void SystemInitHook(void)
{
    /* Initialize MCMGR - low level multicore management library. Call this
       function as close to the reset entry as possible to allow CoreUp event
       triggering. The SystemInitHook() weak function overloading is used in this
       application. */
    (void)MCMGR_EarlyInit();
}

/*!
 * @brief Main function
 */

#define USB_DEVICE_CONFIG_LPCIP3511FS (0U)
#define USB_DEVICE_CONFIG_LPCIP3511HS (1U)

int main(void)
{
    /* Init board hardware.*/
    /* set BOD VBAT level to 1.65V */
    POWER_SetBodVbatLevel(kPOWER_BodVbatLevel1650mv, kPOWER_BodHystLevel50mv, false);
    /* attach main clock divide to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* enable clock for GPIO */
    CLOCK_EnableClock(kCLOCK_Gpio0);
    CLOCK_EnableClock(kCLOCK_Gpio1);

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    SYSCON->AHBMATPRIO|=0x0f;		//CPU0 Code-bus and CPU0 System-bus prio both set to 3, highest

    /* Initialize MCMGR, install generic event handlers */
	CORE0_initMCMGR();
    /* Boot Secondary core application */
    (void)MCMGR_StartCore(kMCMGR_Core1, (void *)(char *)CORE1_BOOT_ADDRESS, 2, kMCMGR_Start_Synchronous);
    PQ_Init(POWERQUAD);
	/* Init Mailbox */
	MAILBOX_Init(MAILBOX);		//must initialize here, if not, mutex will be always "taken"
	/* Enable mailbox interrupt */
	NVIC_EnableIRQ(MAILBOX_IRQn);
    while(1)
    {
    	/* DO NOT DELETE */
    	/* This should be done to let Core1 work */
	GET_CYCLE_COUNTER_Z(CycleCounter1);
	GET_CYCLE_COUNTER_Z(CycleCounter2);
	CycleCounter2-=CycleCounter1;
		/* DO NOT DELETE */
    }
}
