/*
 * GENERATED FILE - DO NOT EDIT
 * Copyright (c) 2008-2013 Code Red Technologies Ltd,
 * Copyright 2015, 2018-2019 NXP
 * (c) NXP Semiconductors 2013-2022
 * Generated linker script file for MIMXRT1176xxxxx
 * Created from memory.ldt by FMCreateLinkMemory
 * Using Freemarker v2.3.30
 * MCUXpresso IDE v11.4.0_prc1 [Build 6157] [2021-05-20] on Apr 6, 2022, 12:37:39 PM
 */

MEMORY
{
  /* Define each memory region */
  BOARD_FLASH (rx) : ORIGIN = 0x30000000, LENGTH = 0x1000000 /* 16M bytes (alias Flash) */  
  SRAM_ITC (rwx) : ORIGIN = 0x0, LENGTH = 0x40000 /* 256K bytes (alias RAM) */  
  SRAM_DTC (rwx) : ORIGIN = 0x20000000, LENGTH = 0x40000 /* 256K bytes (alias RAM2) */  
  SRAM_OC1 (rwx) : ORIGIN = 0x20240000, LENGTH = 0xbfff0 /* 786416 bytes (alias RAM3) */  
  NCACHE_REGION (rwx) : ORIGIN = 0x20300000, LENGTH = 0x40000 /* 256K bytes (alias RAM4) */  
  SRAM_OC_ECC1 (rwx) : ORIGIN = 0x20340000, LENGTH = 0x10000 /* 64K bytes (alias RAM5) */  
  SRAM_OC_ECC2 (rwx) : ORIGIN = 0x20350000, LENGTH = 0x10000 /* 64K bytes (alias RAM6) */  
  BOARD_SDRAM (rwx) : ORIGIN = 0x80000000, LENGTH = 0x4000000 /* 64M bytes (alias RAM7) */  
  RAM_CONVERSA_CONTROL_BLOCK (rwx) : ORIGIN = 0x202ffff0, LENGTH = 0x10 /* 16 bytes (alias RAM8) */  
}

  /* Define a symbol for the top of each memory region */
  __base_BOARD_FLASH = 0x30000000  ; /* BOARD_FLASH */  
  __base_Flash = 0x30000000 ; /* Flash */  
  __top_BOARD_FLASH = 0x30000000 + 0x1000000 ; /* 16M bytes */  
  __top_Flash = 0x30000000 + 0x1000000 ; /* 16M bytes */  
  __base_SRAM_ITC = 0x0  ; /* SRAM_ITC */  
  __base_RAM = 0x0 ; /* RAM */  
  __top_SRAM_ITC = 0x0 + 0x40000 ; /* 256K bytes */  
  __top_RAM = 0x0 + 0x40000 ; /* 256K bytes */  
  __base_SRAM_DTC = 0x20000000  ; /* SRAM_DTC */  
  __base_RAM2 = 0x20000000 ; /* RAM2 */  
  __top_SRAM_DTC = 0x20000000 + 0x40000 ; /* 256K bytes */  
  __top_RAM2 = 0x20000000 + 0x40000 ; /* 256K bytes */  
  __base_SRAM_OC1 = 0x20240000  ; /* SRAM_OC1 */  
  __base_RAM3 = 0x20240000 ; /* RAM3 */  
  __top_SRAM_OC1 = 0x20240000 + 0xbfff0 ; /* 786416 bytes */  
  __top_RAM3 = 0x20240000 + 0xbfff0 ; /* 786416 bytes */  
  __base_NCACHE_REGION = 0x20300000  ; /* NCACHE_REGION */  
  __base_RAM4 = 0x20300000 ; /* RAM4 */  
  __top_NCACHE_REGION = 0x20300000 + 0x40000 ; /* 256K bytes */  
  __top_RAM4 = 0x20300000 + 0x40000 ; /* 256K bytes */  
  __base_SRAM_OC_ECC1 = 0x20340000  ; /* SRAM_OC_ECC1 */  
  __base_RAM5 = 0x20340000 ; /* RAM5 */  
  __top_SRAM_OC_ECC1 = 0x20340000 + 0x10000 ; /* 64K bytes */  
  __top_RAM5 = 0x20340000 + 0x10000 ; /* 64K bytes */  
  __base_SRAM_OC_ECC2 = 0x20350000  ; /* SRAM_OC_ECC2 */  
  __base_RAM6 = 0x20350000 ; /* RAM6 */  
  __top_SRAM_OC_ECC2 = 0x20350000 + 0x10000 ; /* 64K bytes */  
  __top_RAM6 = 0x20350000 + 0x10000 ; /* 64K bytes */  
  __base_BOARD_SDRAM = 0x80000000  ; /* BOARD_SDRAM */  
  __base_RAM7 = 0x80000000 ; /* RAM7 */  
  __top_BOARD_SDRAM = 0x80000000 + 0x4000000 ; /* 64M bytes */  
  __top_RAM7 = 0x80000000 + 0x4000000 ; /* 64M bytes */  
  __base_RAM_CONVERSA_CONTROL_BLOCK = 0x202ffff0  ; /* RAM_CONVERSA_CONTROL_BLOCK */  
  __base_RAM8 = 0x202ffff0 ; /* RAM8 */  
  __top_RAM_CONVERSA_CONTROL_BLOCK = 0x202ffff0 + 0x10 ; /* 16 bytes */  
  __top_RAM8 = 0x202ffff0 + 0x10 ; /* 16 bytes */  
