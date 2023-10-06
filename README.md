# NXP Application Code Hub
[<img src="https://mcuxpresso.nxp.com/static/icon/nxp-logo-color.svg" width="100"/>](https://www.nxp.com)

## Conversa Voice Calling
This project holds the <a href="https://www.nxp.com/design/software/embedded-software/application-software-pack-conversa-voice-calling-on-i-mx-rt1170:APP-SW-PACK-CONVERSA-VOICE">Conversa Voice Calling App SW Pack</a>. This application software pack enables a complete voice call application over USB using NXP’s LPC55S69 MCU and <a href="https://www.nxp.com/design/software/embedded-software/conversa-voice-suite:CONVERSA-VOICE-SUITE">Conversa Voice Suite</a>. <br />This project speeds up the evaluation of Voice over Internet Protocol (VoIP) products and provides sufficient audio quality required to meet the Microsoft Teams specification and receive certification. <br />The ```main_lpc55``` branch contains the conversa voice calling software pack for the LPC55S69. To see other available devices, please check the other branches.<br /><b>NOTE</b>: The Conversa library comes with a 10-minute trial timeout.<br />

<p align="center">
	<img width="200" height="150" src="conversa_voice_calling/conversa_app/images/icon.jpg">
</p>

#### Boards: LPCXpresso55S69
#### Categories: Voice
#### Peripherals: dma, I2S, USB
#### Toolchains: mcux

## Table of Contents
1. [Resources](#step1)
2. [Software](#step1)
3. [Hardware](#step3)
4. [Setup](#step4)
5. [Results](#step5)
6. [FAQs](#step6) 
7. [Support](#step7)
8. [Release Notes](#step8)

## 1. Resources<a name="step1"></a>
* [Application Note 13976](https://www.nxp.com/doc/AN13976) - Covers technical details of the software pack.
* [Lab Guide](https://www.nxp.com/doc/AN13976) and [Video Walkthrough](https://www.nxp.com/design/training/conversa-voice-calling-on-lpc55s69-mcus:TIP-CONVERSA-VOICE-CALLING-LPC55S69) - Walks you through downloading, importing, and running the software pack.
* [Conversa Voice Suite User's Guide](https://www.nxp.com/webapp/Download?colCode=CVSUG) - Provides useful information about the Conversa Voice Suite library and covers technical information about the Conversa Tuning Tool.

## 2. Software<a name="step2"></a>
* [MCUXpresso IDE v11.6.0](https://www.nxp.com/design/software/development-software/mcuxpresso-software-and-tools-/mcuxpresso-integrated-development-environment-ide:MCUXpresso-IDE).
* [Conversa Tuning Tool](https://www.nxp.com/design/software/development-software/mcuxpresso-software-and-tools-/lpcxpresso-boards/application-software-pack-conversa-voice-calling:APP-SW-PACK-CONVERSA-VOICE) for the LPC55S69.

## 3. Hardware<a name="step3"></a>
* [LPCXpresso55S69-EVK](https://www.nxp.com/design/software/development-software/mcuxpresso-software-and-tools-/lpcxpresso-boards/lpcxpresso55s69-development-board:LPC55S69-EVK)
* Two micro USB cables.
* Conversa Hardware mockup for a complete test.

## 4. Setup<a name="step4"></a>

### 4.1 Assemble the Application
You need to have both Git and [West](https://docs.zephyrproject.org/latest/develop/west/index.html) installed, then execute the below commands to gather the complete APP-SW-PACKS/CONVERSA-VOICE-CALLING delivery at branch ```${branch}``` and place it in a folder named ```appswpacks_conversa_voice_calling```. 

Replace ```${branch}``` with another branch within the repo to select the device you would like to use. This can be ```main_lpc55``` for the LPC55S69-EVK. To see other available devices, please check the other branches. 

```
west init -m https://github.com/nxp-appcodehub/ap-conversa-voice-calling --mr ${branch} appswpacks_conversa_voice_calling
cd appswpacks_conversa_voice_calling
west update
```

You can find more information about setup the conversa voice calling application software pack in the Audio Lab Guides included in the [AN13976](https://www.nxp.com/doc/AN13976).

### 4.2 Build and Run the Application
To build and run the application, please refer to the chapter 5. Audio Lab Guides within the AN13976.

## 5. Results<a name="step5"></a>

### Application Overview
This software application pack enables a complete voice call application using NXP’s LPC55S69 MCU and Conversa Voice Suite library for voice processing and echo cancellation.

This project is implemented as a dual-core application. Below is the description of each core application:

Core 0 (Coxtex-M33):
    Handles the execution of Conversa Voice Suite.

Core 1 (Coxtex-M33):
    Handles the initialization of I/O, DMA, USB, and audio Rx/Tx path to start the execution of the selected configuration.

A Inter-CPU mailbox is used for Inter-Process communication between both cores.


  1. **Core 1:** 
   - Handle shell command

		Waits for user input to select one of the software configurations. Type "help" to retrieve the available command:

		With Conversa Voice Suite processing

			"voicecall spswp16k"    => Speakerphone with Conversa software (required voice call LPC55S69 mockup). Teams conferencing device 1.5m. Fs=16kHz
					
		Without Conversa Voice Suite processing

			"voicecall usbswp16k"   => USB microphones and USB speaker. Fs=16kHz
			"voicecall lbswp16k"    => Loop back between microphones and headset + USB microphones. Fs=16kHz
		
		WARNING : FOR lb CONFIGURATION REMOVE SPEAKER JACK CONNECTOR AND PLUG A HEADSET TO AVOID HAWLING EFFECT AND DAMAGE SPEAKER: WARNING
        
		**LPC55S69 mockup** : LPC55S69 EVK (2 microphones used) + TA2024 amplifier + 1 TangBand 2136 speaker.

		**sp** : sp means speaker phone with Conversa software. Conversa Tx output is sent to Tx USB channel 0. Conversa Rx output is sent to the speaker.

		**usb** : USB only use case, no process is running. Tx chain input (microphones) data is sent USB Tx. USB Rx data is sent to speakers. No process is present.

		**lb** : lb means loop back. Tx chain input is sent to speakers and to USB Tx. USB Rx data are not handled. No process is present.

   - Init I2S Tx path for microphones capture
   - Init I2S Rx path + Codec for speaker playback
   - Init USB composite stream for
    - USB Tx path sink audio stream
    - USB Rx path source audio stream
    - USB Conversa serial communication port stream
   - Run audio process loop
    - Wait ISR occurs (I2S microphone and speaker)
	- Get Tx path source data frame n from I2S microphone
	- Get Rx path source data frame n from USB
	- Send Tx path sink data frame n-2 to USB
	- Send Rx path sink data frame n-2 to speaker
	- Check Conversa process frame n-1 on core 0 is finished else MIPS error occurs
	- Send message (MCMGR) to core 0 to enable Conversa process on frame n
	
  2. **Core 0:** 
	- Handles the Conversa Voice Suite execution.


If a conversa voice calling software configuration is selected, the Core 0 will execute the Conversa echo-cancelling audio processing on demand by core 1 message.

**NOTE:** The Conversa library comes with a 10-minute trial timeout.


The image below shows the audio framework architecture.  \
![flowchart](conversa_voice_calling/conversa_app/images/AudioFrameworkArchitecture.png)


## 6. FAQs<a name="step6"></a>

### 6.1 How to get the Conversa HW mockup?
Contact us at voice@nxp.com

### 6.1 How to get an extended timeout Conversa library?
Contact us at voice@nxp.com

### 6.3 Have a question about the Conversa Voice Calling Software Pack?
If you have some question create a ticket to NXP community.

### 6.4 Other Reference Applications
To find another NXP application software packs or demos, visit NXP's [Application Code Hub](https://mcuxpresso.nxp.com/appcodehub).

For SDK examples please go to the [MCUXpresso SDK](https://github.com/nxp-mcuxpresso/mcux-sdk) and get the full delivery to be able to build and run examples that are based on other SDK components.

## 7. Support<a name="step7"></a>
#### Project Metadata
<!----- Boards ----->
[![Board badge](https://img.shields.io/badge/Board-LPCXPRESSO55S69-blue)](https://github.com/search?q=org%3Anxp-appcodehub+LPCXpresso55S69+in%3Areadme&type=Repositories)

<!----- Categories ----->
[![Category badge](https://img.shields.io/badge/Category-VOICE-yellowgreen)](https://github.com/search?q=org%3Anxp-appcodehub+voice+in%3Areadme&type=Repositories)

<!----- Peripherals ----->
[![Peripheral badge](https://img.shields.io/badge/Peripheral-I2S-yellow)](https://github.com/search?q=org%3Anxp-appcodehub+i2s+in%3Areadme&type=Repositories) [![Peripheral badge](https://img.shields.io/badge/Peripheral-DMA-yellow)](https://github.com/search?q=org%3Anxp-appcodehub+dma+in%3Areadme&type=Repositories) [![Peripheral badge](https://img.shields.io/badge/Peripheral-USB-yellow)](https://github.com/search?q=org%3Anxp-appcodehub+usb+in%3Areadme&type=Repositories)

<!----- Toolchains ----->
[![Toolchain badge](https://img.shields.io/badge/Toolchain-MCUXPRESSO%20IDE-orange)](https://github.com/search?q=org%3Anxp-appcodehub+mcux+in%3Areadme&type=Repositories)

Questions regarding the content/correctness of this example can be entered as Issues within this GitHub repository.

>**Warning**: For more general technical questions regarding NXP Microcontrollers and the difference in expected funcionality, enter your questions on the [NXP Community Forum](https://community.nxp.com/)

[![Follow us on Youtube](https://img.shields.io/badge/Youtube-Follow%20us%20on%20Youtube-red.svg)](https://www.youtube.com/@NXP_Semiconductors)
[![Follow us on LinkedIn](https://img.shields.io/badge/LinkedIn-Follow%20us%20on%20LinkedIn-blue.svg)](https://www.linkedin.com/company/nxp-semiconductors)
[![Follow us on Facebook](https://img.shields.io/badge/Facebook-Follow%20us%20on%20Facebook-blue.svg)](https://www.facebook.com/nxpsemi/)
[![Follow us on Twitter](https://img.shields.io/badge/Twitter-Follow%20us%20on%20Twitter-white.svg)](https://twitter.com/NXP)


## 8. Release Notes<a name="step8"></a>
| Version | Description / Update                           | Date                        |
|:-------:|------------------------------------------------|----------------------------:|
| 1.0     | Initial release on Application Code HUb        | July 31<sup>st</sup> 2023   |
