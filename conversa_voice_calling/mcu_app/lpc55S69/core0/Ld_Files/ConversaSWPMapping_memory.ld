/*
 * GENERATED FILE - DO NOT EDIT
 * Copyright (c) 2008-2013 Code Red Technologies Ltd,
 * Copyright 2015, 2018-2019 NXP
 * (c) NXP Semiconductors 2013-2023
 * Generated linker script file for LPC55S69
 * Created from memory.ldt by FMCreateLinkMemory
 * Using Freemarker v2.3.30
 * MCUXpresso IDE v11.6.0 [Build 8187] [2022-07-13] on Nov 17, 2022, 11:41:58 AM
 */

MEMORY
{
  /* Define each memory region */
  PROGRAM_FLASH (rx) : ORIGIN = 0x0, LENGTH = 0x80000 /* 512K bytes (alias Flash) */  
  Flash_BootCore1 (rx) : ORIGIN = 0x80000, LENGTH = 0x1d800 /* 118K bytes (alias Flash2) */  
  Ram0 (rwx) : ORIGIN = 0x20000000, LENGTH = 0x30000 /* 192K bytes (alias RAM) */  
  SRAMX (rwx) : ORIGIN = 0x4000000, LENGTH = 0x8000 /* 32K bytes (alias RAM2) */  
}

  /* Define a symbol for the top of each memory region */
  __base_PROGRAM_FLASH = 0x0  ; /* PROGRAM_FLASH */  
  __base_Flash = 0x0 ; /* Flash */  
  __top_PROGRAM_FLASH = 0x0 + 0x80000 ; /* 512K bytes */  
  __top_Flash = 0x0 + 0x80000 ; /* 512K bytes */  
  __base_Flash_BootCore1 = 0x80000  ; /* Flash_BootCore1 */  
  __base_Flash2 = 0x80000 ; /* Flash2 */  
  __top_Flash_BootCore1 = 0x80000 + 0x1d800 ; /* 118K bytes */  
  __top_Flash2 = 0x80000 + 0x1d800 ; /* 118K bytes */  
  __base_Ram0 = 0x20000000  ; /* Ram0 */  
  __base_RAM = 0x20000000 ; /* RAM */  
  __top_Ram0 = 0x20000000 + 0x30000 ; /* 192K bytes */  
  __top_RAM = 0x20000000 + 0x30000 ; /* 192K bytes */  
  __base_SRAMX = 0x4000000  ; /* SRAMX */  
  __base_RAM2 = 0x4000000 ; /* RAM2 */  
  __top_SRAMX = 0x4000000 + 0x8000 ; /* 32K bytes */  
  __top_RAM2 = 0x4000000 + 0x8000 ; /* 32K bytes */  
