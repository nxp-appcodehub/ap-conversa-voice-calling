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

//fsl driver headers

#include "fsl_debug_console.h"
#include "fsl_power.h"
#include "fsl_mailbox.h"
#include "fsl_shell.h"
#include "fsl_gpio.h"

//Application headers

#include "usb_device_config.h"
#include "mcmgr.h"
#include "appGlobal.h"
#include "commandShell_LPC55S69_CFG1.h"
#include "audioTxRxProcess.h"
#include "Core1MCMGR.h"

// SSRC
#include "SSRC.h"



/*******************************************************************************
 * Macro Definitions
 ******************************************************************************/


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
extern void InitUsbCompositeDevice(void);
/*******************************************************************************
 * Global Variables
 ******************************************************************************/
// Sink RX ping pong buffer
#if RX_PATH_PRESENT
SDK_ALIGN 	( PL_UINT32 g_sinkBuffPingRx	  		  [APP_AUDIO_RX_SAMPLE_PER_FRAME_MAX * APP_AUDIO_RX_CHANNEL_NUMBER_MAX] 	   	, sizeof(PL_UINT16)); // sink buffer (INTERLEAVED OR NOT) ping for the Rx path
SDK_ALIGN 	( PL_UINT32 g_sinkBuffPongRx	  		  [APP_AUDIO_RX_SAMPLE_PER_FRAME_MAX * APP_AUDIO_RX_CHANNEL_NUMBER_MAX] 	   	, sizeof(PL_UINT16)); // sink buffer (INTERLEAVED OR NOT) pong for the Rx path
#endif
// MIC
SDK_ALIGN 	( PL_UINT32 g_sourceRawBuffPingTx	  	  [APP_AUDIO_TX_SAMPLE_PER_FRAME_RDSPMIC * APP_AUDIO_I2S_TX_MAX_MICS_PER_CH] 	, sizeof(PL_UINT16)); // sink buffer (INTERLEAVED OR NOT) ping for the Rx path
SDK_ALIGN 	( PL_UINT32 g_sourceRawBuffPongTx	      [APP_AUDIO_TX_SAMPLE_PER_FRAME_RDSPMIC * APP_AUDIO_I2S_TX_MAX_MICS_PER_CH] 	, sizeof(PL_UINT16)); // sink buffer (INTERLEAVED OR NOT) pong for the Rx path
// USB Circular buffers
SDK_ALIGN 	( PL_INT8 g_USBCircularBufferTX	  	      [APP_USB_TX_CIRCULAR_BUFFER_SIZE_BYTE] 	   	                                , sizeof(PL_UINT16)); // usb tx circular buffer
#if RX_PATH_PRESENT
SDK_ALIGN 	( PL_INT8 g_USBCircularBufferRX	          [APP_USB_RX_CIRCULAR_BUFFER_SIZE_BYTE] 										, sizeof(PL_UINT16)); // usb rx circular buffer
#endif
// Process buffers
#if RX_PATH_PRESENT
SDK_ALIGN 	( PL_INT8 g_processBuff_Rx	              [APP_USB_RX_CHANNEL_MAX][APP_AUDIO_RX_BYTE_PER_FRAME_MAX] 	   		        , sizeof(PL_UINT16)); // usb rx circular buffer
#endif
SDK_ALIGN	( PL_INT8 g_processBuff_Tx		          [APP_USB_TX_CHANNEL_MAX][APP_AUDIO_TX_BYTE_PER_FRAME_MAX]						, sizeof(PL_UINT16)); // Channel by Channel process buffer for the Tx path
// for SSRC use
SDK_ALIGN 	( PL_INT32   g_TXRawBuff	          	  [APP_AUDIO_TX_SAMPLE_PER_FRAME_RDSPMIC * APP_AUDIO_I2S_TX_MAX_MICS_PER_CH] 																	    , sizeof(PL_UINT16)); // usb rx circular buffer
SDK_ALIGN 	( PL_INT32	 SSRC_scratchMem              [APP_AUDIO_TX_SAMPLE_PER_FRAME_RDSPMIC * 6] 					                , sizeof(PL_UINT16)); // SSRC scratch memory

/*******************************************************************************
 * Variables
 ******************************************************************************/
extern PL_INT32 ControlBlockAddr;
extern volatile PL_BOOL						  g_ConversaProcessError;

//this shared var first, then core0 follows accessing it, in one cycle
volatile T_CommonVarSharedByCore0AndCore1 	CommonVarSharedByCore0AndCore1;

// application handle
app_handle_st                             	g_appHandle; 						// general application handle
// Shell task or default
PL_BOOL				                      	s_ShellCommandToBeRun = PL_FALSE;	// Shell task to be run or not

/* SSRC */
SSRC_Scratch_t* 							pScratch;                   		// Pointer to SSRC scratch memory
SSRC_Instance_t 							SSRC_Instance;              		// Allocate memory for the instance
SSRC_Params_t   							SSRC_Params;                		// Memory for init parameters
LVM_INT32       							ScratchSize;                		// The size of the scratch memory
LVM_INT16*      							pInputInScratch;            		// Pointer to input in the scratch buffer
LVM_INT16*      							pOutputInScratch;           		// Pointer to the output in the scratch buffer

/*******************************************************************************
 * Code
 ******************************************************************************/
#if defined(__CC_ARM) || (defined(__ARMCC_VERSION)) || defined(__GNUC__)
int main(void)
#else
void main(void)
#endif
{

	status_t 	   returnStatus_app; 								 // return error from a app sub function
	mcmgr_status_t status;
	PL_UINT32      startupData;
 	shell_status_t shellRetStatus 	   = kStatus_SHELL_Success;      // return error from shell
 	/******************************************************************
	 * RESET
	 *  	- Reset IRQ & Peripherals
	 * 		- Reset Shared structure
	 */
	/* RESET: Reset IRQ & Peripherals*/
    NVIC_ClearPendingIRQ(USB0_IRQn);
    NVIC_ClearPendingIRQ(USB0_NEEDCLK_IRQn);
    NVIC_ClearPendingIRQ(USB1_IRQn);
    NVIC_ClearPendingIRQ(USB1_NEEDCLK_IRQn);
    POWER_DisablePD(kPDRUNCFG_PD_USB0_PHY); /*< Turn on USB0 Phy */
    POWER_DisablePD(kPDRUNCFG_PD_USB1_PHY); /*< Turn on USB1 Phy */
    /* reset the IP to make sure it's in reset state. */
    RESET_PeripheralReset(kUSB0D_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB0HSL_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB0HMR_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB1H_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB1D_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB1_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB1RAM_RST_SHIFT_RSTn);
#if (defined USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS)
	CLOCK_EnableClock(kCLOCK_Usbh1);
	/* Put PHY powerdown under software control */
	*((PL_UINT32 *)(USBHSH_BASE + 0x50)) = USBHSH_PORTMODE_SW_PDCOM_MASK;
	/* According to reference mannual, device mode setting has to be set by access usb host register */
	*((PL_UINT32 *)(USBHSH_BASE + 0x50)) |= USBHSH_PORTMODE_DEV_ENABLE_MASK;
	/* enable usb1 host clock */
	CLOCK_DisableClock(kCLOCK_Usbh1);
#endif
#if (defined USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS)
	POWER_DisablePD(kPDRUNCFG_PD_USB0_PHY); /*< Turn on USB Phy */
	//CLOCK_SetClkDiv(kCLOCK_DivUsb0Clk, 1, false);
	//CLOCK_AttachClk(kFRO_HF_to_USB0_CLK);
	/* enable usb0 host clock */
	CLOCK_EnableClock(kCLOCK_Usbhsl0);
	/*According to reference mannual, device mode setting has to be set by access usb host register */
	*((PL_UINT32 *)(USBFSH_BASE + 0x5C)) |= USBFSH_PORTMODE_DEV_ENABLE_MASK;
	/* disable usb0 host clock */
	CLOCK_DisableClock(kCLOCK_Usbhsl0);
#endif
#if (defined USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS)
	CLOCK_AttachClk(kPLL0_to_CTIMER0);
#elif (defined USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS)
	CLOCK_AttachClk(kPLL0_to_CTIMER0);
	CLOCK_AttachClk(kFRO_HF_to_CTIMER1);
#endif
	/* RESET:  Reset the shared structure*/
	memset((void *)&CommonVarSharedByCore0AndCore1,
		   0,
		   sizeof(CommonVarSharedByCore0AndCore1));
	/******************************************************************
	 * INIT
	 *      - Init Board & Clock
	 * 		- Init  default audio Handle
	 * 		- Init Shell (only if SHELL_COMMAND_PRESENT==1)
	 * 		- Init MCMGR & Mailbox ( only if Core0 is used )
	 * 		- Init SW Process
	 * 		- Init Audio Use Case
	 *
	 */
 	/* INIT : Board & Clock */
	returnStatus_app = initBoardClock();
    PRINTF("\r\n");
	PRINTF("**********************************\r\n");
	PRINTF("Application: Init board & clock\r\n");
	if (returnStatus_app != kStatus_Success)
	{
		PRINTF("=> FAIL : fail to init Board & clock\r\n");
		while (1);
	}
    /* INIT: set audio structure with default value */
	returnStatus_app =  AUDIO_SetHandleDefault(&g_appHandle.audioDefinition); // set default value to g_appHandle.audioDefinition
	if (returnStatus_app != kStatus_Success)
	{
		PRINTF("=> FAIL: Fail to init default audio parameters\r\n");
		while (1);
	}
	PRINTF("**********************************\r\n");
	PRINTF("\n**********************************\r\n");
	PRINTF("Platform supported: %s\r\n",APP_PLATFORM_TEXT);
	PRINTF("Audio frame work version: %d.%d.%d\r\n",APP_VERSION_MAJOR,APP_VERSION_MINOR,APP_VERSION_PATCH);
	PRINTF("**********************************\r\n\n");
#ifdef SHELL_COMMAND_PRESENT
		s_ShellCommandToBeRun = PL_TRUE; // Shell command to be run by default
	/* Shell command is present by default
	 *  - USER_BUTTON is not pressed => launch command line interface
	 *  - USER_BUTTON is pressed     => launch default configuration
	 */
#if (APP_PLATFORM == APP_PL_LPC55S69EVK)
		if(    GPIO_PinRead(GPIO, 1, BOARD_USER_BUTTON) == 0 )
		{
			// USER_BUTTON is pressed down --- GPIO level == Low
			PRINTF("%s is pressed => Force to launch automatically the default configuration\r\n\n", BOARD_USER_BUTTON_NAME);
			s_ShellCommandToBeRun = PL_FALSE;
		}
		else
		{
			s_ShellCommandToBeRun = PL_TRUE;
			PRINTF("\r\n");
			PRINTF("Enter a command or 'help'\r\n");
		}
#endif /*(APP_PLATFORM == APP_PL_LPC55S69EVK)*/
#else
		if(    GPIO_PinRead(GPIO, 1, BOARD_USER_BUTTON) == 0 )
		{
			// USER_BUTTON is pressed down --- GPIO level == Low
			PRINTF("\r\n");
			PRINTF("Application is running: Enter a command or 'help'\r\n");
			s_ShellCommandToBeRun = PL_TRUE;
		}
		else
		{
			s_ShellCommandToBeRun = PL_FALSE;
			PRINTF("launch automatically the default configuration\r\n\n");
		}

#endif
#if CONVERSA_PRESENT
	/* Init  MCMGR + Mailbox  on Core1*/
	CORE1_initMCMGR();
	MAILBOX_Init(MAILBOX);		//must initialize here, if not, mutex will be always "taken"
#endif /* CONVERSA_PRESENT */
	if ( s_ShellCommandToBeRun == PL_TRUE )  // Launch the shell command
	{
		/* INIT: Shell Command */
		InitShell();
	}
	else /* No shell command, launch the SHELL_COMMAND_DEFAULT_CONFIGURATION */
	{
		PRINTF("Application: Auto launch the default configuration: voicecall %s\r\n",SHELL_COMMAND_DEFAULT_CONFIGURATION);
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
		strcpy(&argvShell[1][0],SHELL_COMMAND_DEFAULT_CONFIGURATION);
		shellTxRxPathDef(&shellHandleShell,  argcShell,  pp_argvShell);
	}
	/* Wait until Audio path is defined */
	while (g_appHandle.audioDefinition.audioPathIsDefined!=PL_TRUE);
	/* INIT: Init SW Process */
	if(g_appHandle.audioDefinition.ConfigWithConversa == PL_TRUE)
	{
		/*
		 * Init CRC engine (for tuning tool purpose)
		 */
		Init_CrcEngine();

		/* Send Shared structure Address to Core0*/
		returnStatus_app = CORE1_sendSharedStructAddress((PL_UINT32)&CommonVarSharedByCore0AndCore1);
		if (returnStatus_app != kStatus_Success)
		{
			PRINTF("FAIL: Fail to send shared structure address \r\n");
			while(1);
		}
		/*send Audio path to shared structure */
		CommonVarSharedByCore0AndCore1.audioDefinition = g_appHandle.audioDefinition;
		/* Ask Core0 to create Conversa */
		MCMGR_TriggerEvent(kMCMGR_Core0_AskConversaCreate, (uint16_t) 0);
		/* Wait until Core0 create Conversa */
		while(g_ConversaIsInit != PL_TRUE);

		ControlBlockAddr=CommonVarSharedByCore0AndCore1.audioDefinition.swIpConversa_handle.conversaDataControlAddress;
	}
	/* INIT: Init Audio Use Case */
	returnStatus_app = initAudioUseCase( &g_appHandle.audioDefinition );
	if (returnStatus_app != kStatus_Success)
	{
		PRINTF("FAIL: Fail to init the hardware\r\n");
		while(1);
	}
	/******************************************************************
	 * START
	 *      - Start Audio capture ( DMA & I2S transfer)
	 * 		- Start Main Loop

	 */
	/* Wait until Audio is init */
	while (g_appHandle.audioDefinition.audioPathIsInit !=PL_TRUE);
	/* START: Start Audio Capture */
	PRINTF("**********************************\r\n");
	PRINTF("Application is running ...\r\n");
	PRINTF("**********************************\r\n");
	returnStatus_app = initStartAudioCapture(&g_appHandle.audioDefinition);
	if (returnStatus_app != kStatus_Success)
	{
		PRINTF("FAIL: Fail to start Audio capture\r\n");
		while(1);
	}
   /* START: Start Main loop */

	while (1)
	{
		/*
		 * Voicecall process
		 */
		/* Wait until Event Group appears */

		while (  (g_appHandle.audioDefinition.audioWorkFlow_handle.pingPongTxStatus != g_appHandle.audioDefinition.audioWorkFlow_handle.currentProcessEventGroup_wait)
#if RX_PATH_PRESENT
			   ||(g_appHandle.audioDefinition.audioWorkFlow_handle.pingPongRxStatus != g_appHandle.audioDefinition.audioWorkFlow_handle.currentProcessEventGroup_wait)
#endif
		);
#ifdef MIPS_MEASURE_GPIO
		GPIO_0_Up();
#endif
		if (g_ConversaProcessError == PL_FALSE)
		{
			audioTxRxProcess(&g_appHandle.audioDefinition);
		}
#ifdef MIPS_MEASURE_GPIO
		GPIO_0_Dn();
#endif
		/*
		 *  User Process can be added here
		 */

	}


}









