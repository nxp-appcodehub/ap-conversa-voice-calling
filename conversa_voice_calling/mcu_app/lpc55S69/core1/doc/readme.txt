Overview
========
The Multicore Conversa voice calling Software Pack provide voicecall speaker phone over IP use case based on voicec call mockup ( LPC55S69 MCU + I2S retune DSP microphone board + TB2136 speaker).

It involves NXP Conversa library which handle the audio Tx/Rx processing.

It permits to speed up voicecall product devellopment and guarantee audio quality with Microsoft Teams specification laboratory validation.

Toolchain supported
===================
- MCUXpresso  11.6.0

Hardware requirements
=====================
- 2 Mini/micro USB cable
- Conversa voicecall LPC55S69 mockup
   Content:
     - LPC55S69 EVK board
	 - External RetuneDSP microphones board
     - An external stereo amplifier
     - 1 Tangband 2136 speakers
   Use case:
     - Run seakerphone voicecall with conversa enabled
     - Dump raw microphones signal and conversa Tx output
     - Play audio signal to the speaker through USB connection
   Limitation:
     - To run loop back mode you need to use a headset instead of speaker to avoid howling effect
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
 Connect USB cable to USB connector P9:
   - Audio RX 2 channel
   - Audio TX 4 channel
   - Conversa tuning tool (required dedicated software install on your PC)

Connect USB cable to CLI USB connector P6:
   - Command line interface
   - MCU link debugger

Open a serial terminal with the following settings
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control

Download the program to the target board.

Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
Refer to Software pack lab guide, if not yet available :
   - type "help"
   - type a "voicecall XXX" to run a use case
   
Core1: 	Handle the shell command, the audio processing loop and the ISR (I2S and USB)
		Handle synchronization with core 1 by message (MCMGR) to run Conversa algorithm