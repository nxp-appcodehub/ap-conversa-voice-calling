/*
 * Copyright 2020-2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>

//general
#include "PL_platformTypes.h"
#include "appGlobal.h"
#include "main.h"

// init
#include "init.h"
#include "audio.h"

// shell
#include "commandShellTask.h"

// Shell ack for debug
#include "commandShell.h"
#include "fsl_shell.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define SHELL_TASK_SW_GPIO         BOARD_USER_BUTTON_GPIO
#define SHELL_TASK_SW_GPIO_PIN     BOARD_USER_BUTTON_GPIO_PIN
#define SHELL_TASK_SW_NAME         BOARD_USER_BUTTON_NAME

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
#ifdef CONVERSA_PRESENT
	SDK_ALIGN	( 	PL_INT8 __attribute__((section(".data.$SRAM_OC1")))    g_conversaMemory		[CONVERSA_MEMORY_BYTE] 	            , sizeof(PL_UINT32)); // memory reserved for conversa
#endif

// Source TX ping pong buffer
SDK_ALIGN 	(  	PL_INT8 __attribute__((section(".data.$SRAM_DTC"))) g_sourceRawBuffPingTx	[APP_AUDIO_PDM_CHANNEL_NUMBER_MAX * APP_AUDIO_TX_RAW_BUF_BYTE_PER_FRAME_MAX]   , sizeof(PL_UINT32)); // RAW (INTERLEAVED OR NOT) source buffer ping for the Tx path
SDK_ALIGN 	(  	PL_INT8 __attribute__((section(".data.$SRAM_DTC"))) g_sourceRawBuffPongTx	[APP_AUDIO_PDM_CHANNEL_NUMBER_MAX * APP_AUDIO_TX_RAW_BUF_BYTE_PER_FRAME_MAX]   , sizeof(PL_UINT32)); // RAW (INTERLEAVED OR NOT) source buffer pong for the Tx path

// Source RX ping pong buffer
SDK_ALIGN 	(  	PL_INT8 __attribute__((section(".data.$SRAM_DTC"))) g_sourceRawBuffRx 		[APP_AUDIO_RX_CHANNEL_NUMBER_MAX * APP_AUDIO_RX_BYTE_PER_FRAME_MAX] 		   , sizeof(PL_UINT32)); // RAW (INTERLEAVED OR NOT) source buffer for the Rx path

// Sink RX ping pong buffer
SDK_ALIGN 	(  	PL_INT8 __attribute__((section(".data.$SRAM_DTC"))) g_sinkBuffPingRx		[APP_AUDIO_RX_CHANNEL_NUMBER_MAX * APP_AUDIO_RX_BYTE_PER_FRAME_MAX] 		   , sizeof(PL_UINT32)); // sink buffer ping for the Rx path
SDK_ALIGN 	(  	PL_INT8 __attribute__((section(".data.$SRAM_DTC"))) g_sinkBuffPongRx		[APP_AUDIO_RX_CHANNEL_NUMBER_MAX * APP_AUDIO_RX_BYTE_PER_FRAME_MAX] 		   , sizeof(PL_UINT32)); // sink buffer pong for the Rx path

// Process buffer Tx and Rx
SDK_ALIGN	(	PL_INT8 __attribute__((section(".data.$SRAM_DTC"))) g_processBuff_Tx		[APP_AUDIO_TX_CHANNEL_NUMBER_MAX] [APP_AUDIO_TX_BYTE_PER_FRAME_MAX]			   , sizeof(PL_UINT32)); // Channel by Channel process buffer for the Tx path
SDK_ALIGN	(	PL_INT8 __attribute__((section(".data.$SRAM_DTC"))) g_processBuff_Rx		[APP_AUDIO_RX_CHANNEL_NUMBER_MAX] [APP_AUDIO_RX_BYTE_PER_FRAME_MAX]		       , sizeof(PL_UINT32)); // Channel by Channel process buffer for the Rx path

/* USB Buffer */
// USB capture buffer
SDK_ALIGN 	(  	PL_INT8 __attribute__((section(".data.$SRAM_DTC"))) g_USBBufferTx			[APP_USB_TX_CHANNEL_MAX] [APP_AUDIO_TX_BYTE_PER_FRAME_MAX] 		   			   , sizeof(PL_UINT32)); // buffer used by the USB to store data to transmit to the USB Tx circular buffer at end of audio process loop

// USB circular buffer
SDK_ALIGN	(	PL_INT8 __attribute__((section(".data.$SRAM_DTC"))) g_USBCircularBufferTX   [APP_USB_TX_CIRCULAR_BUFFER_SIZE_BYTE]									       , sizeof(PL_UINT32)); // circular buffer used by the USB software
SDK_ALIGN	(	PL_INT8 __attribute__((section(".data.$SRAM_DTC"))) g_USBCircularBufferRX   [APP_USB_RX_CIRCULAR_BUFFER_SIZE_BYTE]									       , sizeof(PL_UINT32)); // circular buffer used by the USB software

// application handle
app_handle_st g_appHandle; // general application handle

// Others

// SAI
sai_edma_handle_t	g_saiTxEdmaHandle;
edma_handle_t		g_dmaSaiTxHandle;			 	// Edma variables for SAI TX

// Shell task or default
PL_BOOL				s_ShellTaskToBeRun = PL_FALSE;	// Shell task to be run or not
// debug
// PL_UINT32 g_iDebug1;

/*
 *  Tx Rx synchro
 */
volatile PL_UINT32 g_startMechanismTxRx = 0;							// to synchronise Rx and Tx at start

/*******************************************************************************
 * Code
 ******************************************************************************/
/*
 * @brief Application initialization function.
 *
 * This function initializes the application.
 *
 * @return None.
 */
int main(void)
{
	status_t 	   returnStatus_app; 								 // return error from a app sub function
	shell_status_t shellRetStatus 	   = kStatus_SHELL_Success;      // return error from shell
	BaseType_t 	   returnStatus_task; 								 //task return status

    /*
     * INIT
     */
    returnStatus_app = initBoardClock();

    PRINTF("\r\n");
    PRINTF("**********************************\r\n");
    PRINTF("Application: Init board & clock\r\n");
    if (returnStatus_app != kStatus_Success)
    {
        PRINTF("=> FAIL\r\n");
        while (1);
    }

	// Initialize OSA
	OSA_Init();

	/*
	 *  SHELL TASK OR DEFAULT COMMAND
	 *     - Scan SWITCH and Compile flag to determine if SHELL CMD is handle by a task or not
	 */
	PRINTF("**********************************\r\n");
	#ifdef SHELL_TASK_PRESENT
   		s_ShellTaskToBeRun = PL_TRUE; // Shell task to be run by default
    #else   // NOT SHELL_TASK_PRESENT
    	#if (APP_PLATFORM == APP_PL_RT1170EVK)
    		s_ShellTaskToBeRun = PL_FALSE; // Shell task disable and autolaunch default configuration by default
    		if ( GPIO_PinRead(SHELL_TASK_SW_GPIO, SHELL_TASK_SW_GPIO_PIN) == 1 ) // If SW7 (USER_BUTTON) is NOT pressed
    		{
    			s_ShellTaskToBeRun = PL_FALSE;
    		}
    		else												  				// Else SW7 (USER_BUTTON) is pressed
    		{
    			// Shell task will be launch in blocking mode
    			PRINTF("%s is pressed: Force to launch the shell task (blocking mode)\r\n\n", SHELL_TASK_SW_NAME);
    			s_ShellTaskToBeRun = PL_TRUE;
    		}
    	#endif // end (APP_PLATFORM == APP_PL_RT1170EVK)
    #endif // SHELL_TASK_PRESENT

	PRINTF("\n**********************************\r\n");
	PRINTF("Platform supported: %s\r\n",APP_PLATFORM_TEXT);
	PRINTF("Audio frame work version: %d.%d.%d\r\n",APP_VERSION_MAJOR,APP_VERSION_MINOR,APP_VERSION_PATCH);
	PRINTF("**********************************\r\n\n");

    /*
     *  Init audio structure with default value
     */
    returnStatus_app =  AUDIO_SetHandleDefault(&g_appHandle.audioDefinition); // set default value to g_appHandle.audioDefinition
    if (returnStatus_app != kStatus_Success)
    {
        PRINTF("=> FAIL to init default audio parameters\r\n");
        while (1);
    }

	/*
	 *  SHELL handle
	 */
    if ( s_ShellTaskToBeRun == PL_TRUE )  // Launch the shell task
    {
    	/* Create task SHELL COMMAND */
		PRINTF("Application: Create Shell command task\r\n");
		returnStatus_task = xTaskCreate(	APP_Shell_Task,
											"Shell Task",
											APP_SHELL_TASK_STACK_SIZE,
											&g_appHandle,
											APP_SHELL_TASK_PRIORITY,
											&g_appHandle.shell_task_handle) ;
		if (returnStatus_task != pdPASS)
		{
			PRINTF("=> FAIL\r\n");
			while (1);
		}
	    PRINTF("\r\n");
	    PRINTF("Application is running: Enter a command or 'help'\r\n");
    }
	else 									// launch the default configuration
	{
		PRINTF("Application: Auto launch the default configuration: voicecall %s\r\n",SHELL_TASK_DEFAULT_CONFIGURATION);
		shell_handle_t shellHandleShell;
		int32_t argcShell = 9;
		int32_t iShell;
		char *pp_argvShell[20];
		char argvShell[20][20];

		for (iShell=1; iShell < argcShell; iShell ++)
		{
			pp_argvShell[iShell] = (char *)&argvShell[iShell];
		}

		// default configuration
		strcpy(&argvShell[1][0],SHELL_TASK_DEFAULT_CONFIGURATION);
		shellRetStatus = shellTxRxPathDef(&shellHandleShell,  argcShell,  pp_argvShell);

		PRINTF("\r\n");
		if (shellRetStatus == kStatus_SHELL_Success)
		{
			PRINTF("Application is running\r\n");
		}
		else
		{
			PRINTF("FAIL: Application is NOT running\r\n");
		}
	}
    PRINTF("**********************************\r\n");


    /*
     *  Run RTOS scheduler
     */
    vTaskStartScheduler(); // Launch the scheduler

    /*
     *  Never reach this statement
     */
    return 0;
}

/**
 * @brief Loop forever if stack overflow is detected.
 *
 * If configCHECK_FOR_STACK_OVERFLOW is set to 1,
 * this hook provides a location for applications to
 * define a response to a stack overflow.
 *
 * Use this hook to help identify that a stack overflow
 * has occurred.
 *
 */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    portDISABLE_INTERRUPTS();

    /* Loop forever */
    for (;;)
        ;
}

/**
 * @brief Warn user if pvPortMalloc fails.
 *
 * Called if a call to pvPortMalloc() fails because there is insufficient
 * free memory available in the FreeRTOS heap.  pvPortMalloc() is called
 * internally by FreeRTOS API functions that create tasks, queues, software
 * timers, and semaphores.  The size of the FreeRTOS heap is set by the
 * configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h.
 *
 */
void vApplicationMallocFailedHook()
{
    PRINTF(("ERROR: Malloc failed to allocate memory\r\n"));

    /* Loop forever */
    for (;;)
        ;
}
