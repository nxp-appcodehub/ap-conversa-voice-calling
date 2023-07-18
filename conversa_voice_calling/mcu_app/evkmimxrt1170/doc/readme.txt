Overview
========
Goal of this code is to provide voicecall speaker phone over IP use case based on RT1170 MCU.
It involves NXP Conversa library which handle the audio Tx/Rx processing.

It permits to speed up voicecall product devellopment and guarantee audio quality with Microsoft Teams specification laboratory validation.

Hardware requirements
=====================
2 hardware setups can be use:

HW setup 1: Conversa voicecall RT1170 mockup
   Content:
     - RT1170 EVK board with its embedded PDM microphones 
     - An external stereo amplifier
     - 1 Tangband 2136 speakers
   Use case:
     - Run seakerphone voicecall with conversa enabled
     - Dump raw microphones signal and conversa Tx output
     - Play audio signal to the speaker through USB connection
   Limitation:
     - Can't Run loop back mode due to Larsen effect

HW setup 2: RT1170EVK + headset
   Content:
     - RT1170 EVK board with its embedded PDM microphones 
     - A headset connected to the audio output jack connector
   Use case:
     - Run headset voicecall with or without conversa enabled
     - Run loop back mode wich permits to listen microphone signal in the headset (no conversa process)
     - Dump raw microphones signal and conversa Tx output
     - Play audio signal to the headset through USB connection
   Limitation:
     - Conversa can be enabled but effect is limited because of the headset usage. Use external speakers need a specific conversa echo path tuning.

Board settings
==============
Connect USB cable to USB 1 connector:
   - Audio RX 2 channel
   - Audio TX 4 channel
   - Conversa tuning tool (required dedicated software install on your PC)

Connect USB cable to CLI USB connector:
   - Enable command line interface
   - Enable MCU link debugger

Running the demo
================
Refer to Software pack lab guide 

