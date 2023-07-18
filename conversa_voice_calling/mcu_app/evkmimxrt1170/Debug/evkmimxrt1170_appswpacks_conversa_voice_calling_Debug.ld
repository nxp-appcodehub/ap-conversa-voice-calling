/*
 * GENERATED FILE - DO NOT EDIT
 * Copyright (c) 2008-2013 Code Red Technologies Ltd,
 * Copyright 2015, 2018-2019 NXP
 * (c) NXP Semiconductors 2013-2023
 * Generated linker script file for MIMXRT1176xxxxx
 * Created from linkscript.ldt by FMCreateLinkLibraries
 * Using Freemarker v2.3.30
 * MCUXpresso IDE v11.3.0 [Build 5222] [2021-01-11] on Apr 4, 2022 5:09:12 PM
 */

INCLUDE "evkmimxrt1170_appswpacks_conversa_voice_calling_library.ld"
INCLUDE "evkmimxrt1170_appswpacks_conversa_voice_calling_memory.ld"

ENTRY(ResetISR)

SECTIONS
{
     /* Image Vector Table and Boot Data for booting from external flash */
    .boot_hdr : ALIGN(4)
    {
        FILL(0xff)
        . = 0x400 ;
        __boot_hdr_start__ = ABSOLUTE(.) ;
        KEEP(*(.boot_hdr.conf))
        . = 0x1000 ;
        __boot_hdr_ivt_loadaddr__  =  ABSOLUTE(.) ;
        KEEP(*(.boot_hdr.ivt))
        . = 0x1020 ;
        __boot_hdr_boot_data_loadaddr__ = ABSOLUTE(.) ;
        KEEP(*(.boot_hdr.boot_data))
        . = 0x1030 ;
        __boot_hdr_dcd_loadaddr__ = ABSOLUTE(.) ;
        KEEP(*(.boot_hdr.dcd_data))
        __boot_hdr_end__ = ABSOLUTE(.) ;
        . = 0x2000 ;
    } >BOARD_FLASH


    /* MAIN TEXT SECTION */
    .text : ALIGN(4)
    {
        FILL(0xff)
        __vectors_start__ = ABSOLUTE(.) ;
        KEEP(*(.isr_vector))
        /* Global Section Table */
        . = ALIGN(4) ;
        __section_table_start = .;
        __data_section_table = .;
        LONG(LOADADDR(.data_RAM));
        LONG(    ADDR(.data_RAM));
        LONG(  SIZEOF(.data_RAM));
        LONG(LOADADDR(.data));
        LONG(    ADDR(.data));
        LONG(  SIZEOF(.data));
        LONG(LOADADDR(.data_RAM3));
        LONG(    ADDR(.data_RAM3));
        LONG(  SIZEOF(.data_RAM3));
        LONG(LOADADDR(.data_RAM4));
        LONG(    ADDR(.data_RAM4));
        LONG(  SIZEOF(.data_RAM4));
        LONG(LOADADDR(.data_RAM5));
        LONG(    ADDR(.data_RAM5));
        LONG(  SIZEOF(.data_RAM5));
        LONG(LOADADDR(.data_RAM6));
        LONG(    ADDR(.data_RAM6));
        LONG(  SIZEOF(.data_RAM6));
        LONG(LOADADDR(.data_RAM7));
        LONG(    ADDR(.data_RAM7));
        LONG(  SIZEOF(.data_RAM7));
        __data_section_table_end = .;
        __bss_section_table = .;
        LONG(    ADDR(.bss_RAM));
        LONG(  SIZEOF(.bss_RAM));
        LONG(    ADDR(.bss));
        LONG(  SIZEOF(.bss));
        LONG(    ADDR(.bss_RAM3));
        LONG(  SIZEOF(.bss_RAM3));
        LONG(    ADDR(.bss_RAM4));
        LONG(  SIZEOF(.bss_RAM4));
        LONG(    ADDR(.bss_RAM5));
        LONG(  SIZEOF(.bss_RAM5));
        LONG(    ADDR(.bss_RAM6));
        LONG(  SIZEOF(.bss_RAM6));
        LONG(    ADDR(.bss_RAM7));
        LONG(  SIZEOF(.bss_RAM7));
        __bss_section_table_end = .;
        __section_table_end = . ;
        /* End of Global Section Table */

        *(.after_vectors*)

    } > BOARD_FLASH

    .text : ALIGN(4)
    {
     	/* exlude following element */
       *(EXCLUDE_FILE (*libRdspConversa.a *libarm_cortexM7lfsp_math.a) .text*)
       *(EXCLUDE_FILE (*libRdspConversa.a) .rodata*)

       /* *(.text*) */

       KEEP(*freertos*/tasks.o(.rodata*)) /* FreeRTOS Debug Config */

       /*  *(.rodata .rodata.* .constdata .constdata.*) */

       . = ALIGN(4);
    } > BOARD_FLASH
 
    _etext = .;
        
    /* DATA section for SRAM_ITC */

    .data_RAM : ALIGN(4)
    {
        FILL(0xff)
        PROVIDE(__start_data_RAM = .) ;
        PROVIDE(__start_data_SRAM_ITC = .) ;
        *(.ramfunc.$RAM)
        *(.ramfunc.$SRAM_ITC)
        *(.data.$RAM)
        *(.data.$SRAM_ITC)
        *(.data.$RAM.*)
        *(.data.$SRAM_ITC.*)
        *libRdspConversa.a:*(.text .text* .text.*)
        *libRdspConversa.a:*(.rodata*)
        *libarm_cortexM7lfsp_math.a:*(.text .text* .text.*)
        . = ALIGN(4) ;
        PROVIDE(__end_data_RAM = .) ;
        PROVIDE(__end_data_SRAM_ITC = .) ;
     } > SRAM_ITC AT>BOARD_FLASH

    /* DATA section for SRAM_OC1 */

    .data_RAM3 : ALIGN(4)
    {
        FILL(0xff)
        PROVIDE(__start_data_RAM3 = .) ;
        PROVIDE(__start_data_SRAM_OC1 = .) ;
        *(.ramfunc.$RAM3)
        *(.ramfunc.$SRAM_OC1)
        *(.data.$RAM3)
        *(.data.$SRAM_OC1)
        *(.data.$RAM3.*)
        *(.data.$SRAM_OC1.*)
        . = ALIGN(4) ;
        PROVIDE(__end_data_RAM3 = .) ;
        PROVIDE(__end_data_SRAM_OC1 = .) ;
     } > SRAM_OC1 AT>BOARD_FLASH

    /* DATA section for NCACHE_REGION */

    .data_RAM4 : ALIGN(4)
    {
        FILL(0xff)
        PROVIDE(__start_data_RAM4 = .) ;
        PROVIDE(__start_data_NCACHE_REGION = .) ;
        *(.ramfunc.$RAM4)
        *(.ramfunc.$NCACHE_REGION)
        *(.data.$RAM4)
        *(.data.$NCACHE_REGION)
        *(.data.$RAM4.*)
        *(.data.$NCACHE_REGION.*)
        . = ALIGN(4) ;
        PROVIDE(__end_data_RAM4 = .) ;
        PROVIDE(__end_data_NCACHE_REGION = .) ;
     } > NCACHE_REGION AT>BOARD_FLASH

    /* DATA section for SRAM_OC_ECC1 */

    .data_RAM5 : ALIGN(4)
    {
        FILL(0xff)
        PROVIDE(__start_data_RAM5 = .) ;
        PROVIDE(__start_data_SRAM_OC_ECC1 = .) ;
        *(.ramfunc.$RAM5)
        *(.ramfunc.$SRAM_OC_ECC1)
        *(.data.$RAM5)
        *(.data.$SRAM_OC_ECC1)
        *(.data.$RAM5.*)
        *(.data.$SRAM_OC_ECC1.*)
        . = ALIGN(4) ;
        PROVIDE(__end_data_RAM5 = .) ;
        PROVIDE(__end_data_SRAM_OC_ECC1 = .) ;
     } > SRAM_OC_ECC1 AT>BOARD_FLASH

    /* DATA section for SRAM_OC_ECC2 */

    .data_RAM6 : ALIGN(4)
    {
        FILL(0xff)
        PROVIDE(__start_data_RAM6 = .) ;
        PROVIDE(__start_data_SRAM_OC_ECC2 = .) ;
        *(.ramfunc.$RAM6)
        *(.ramfunc.$SRAM_OC_ECC2)
        *(.data.$RAM6)
        *(.data.$SRAM_OC_ECC2)
        *(.data.$RAM6.*)
        *(.data.$SRAM_OC_ECC2.*)
        . = ALIGN(4) ;
        PROVIDE(__end_data_RAM6 = .) ;
        PROVIDE(__end_data_SRAM_OC_ECC2 = .) ;
     } > SRAM_OC_ECC2 AT>BOARD_FLASH

    /* DATA section for RAM_CONVERSA_CONTROL_BLOCK */

    .data_RAM7 : ALIGN(4)
    {
        FILL(0xff)
        PROVIDE(__start_data_RAM7 = .) ;
        PROVIDE(__start_data_RAM_CONVERSA_CONTROL_BLOCK = .) ;
        *(.ramfunc.$RAM7)
        *(.ramfunc.$RAM_CONVERSA_CONTROL_BLOCK)
        *(.data.$RAM7)
        *(.data.$RAM_CONVERSA_CONTROL_BLOCK)
        *(.data.$RAM7.*)
        *(.data.$RAM_CONVERSA_CONTROL_BLOCK.*)
        . = ALIGN(4) ;
        PROVIDE(__end_data_RAM7 = .) ;
        PROVIDE(__end_data_RAM_CONVERSA_CONTROL_BLOCK = .) ;
     } > RAM_CONVERSA_CONTROL_BLOCK AT>BOARD_FLASH

    /* MAIN DATA SECTION */
    .uninit_RESERVED (NOLOAD) : ALIGN(4)
    {
        _start_uninit_RESERVED = .;
        KEEP(*(.bss.$RESERVED*))
       . = ALIGN(4) ;
        _end_uninit_RESERVED = .;
    } > SRAM_DTC AT> SRAM_DTC

    /* Main DATA section (SRAM_DTC) */
    .data : ALIGN(4)
    {
       FILL(0xff)
       _data = . ;
       PROVIDE(__start_data_RAM2 = .) ;
       PROVIDE(__start_data_SRAM_DTC = .) ;
       *(vtable)
       *(.ramfunc*)
       KEEP(*(CodeQuickAccess))
       KEEP(*(DataQuickAccess))
       *(RamFunction)
       *(NonCacheable.init)
       *(.data*)
       . = ALIGN(4) ;
       _edata = . ;
       PROVIDE(__end_data_RAM2 = .) ;
       PROVIDE(__end_data_SRAM_DTC = .) ;
    } > SRAM_DTC AT>BOARD_FLASH

    /* BSS section for SRAM_ITC */
    .bss_RAM : ALIGN(4)
    {
       PROVIDE(__start_bss_RAM = .) ;
       PROVIDE(__start_bss_SRAM_ITC = .) ;
       *(.bss.$RAM)
       *(.bss.$SRAM_ITC)
       *(.bss.$RAM.*)
       *(.bss.$SRAM_ITC.*)
       . = ALIGN (. != 0 ? 4 : 1) ; /* avoid empty segment */
       PROVIDE(__end_bss_RAM = .) ;
       PROVIDE(__end_bss_SRAM_ITC = .) ;
    } > SRAM_ITC AT> SRAM_ITC

    /* BSS section for SRAM_OC1 */
    .bss_RAM3 : ALIGN(4)
    {
       PROVIDE(__start_bss_RAM3 = .) ;
       PROVIDE(__start_bss_SRAM_OC1 = .) ;
       *heap_4.o(.bss .bss*)
       *(.bss.$RAM3)
       *(.bss.$SRAM_OC1)
       *(.bss.$RAM3.*)
       *(.bss.$SRAM_OC1.*)
       . = ALIGN (. != 0 ? 4 : 1) ; /* avoid empty segment */
       PROVIDE(__end_bss_RAM3 = .) ;
       PROVIDE(__end_bss_SRAM_OC1 = .) ;
    } > SRAM_OC1 AT> SRAM_OC1

    /* BSS section for NCACHE_REGION */
    .bss_RAM4 : ALIGN(4)
    {
       PROVIDE(__start_bss_RAM4 = .) ;
       PROVIDE(__start_bss_NCACHE_REGION = .) ;
       *(.bss.$RAM4)
       *(.bss.$NCACHE_REGION)
       *(.bss.$RAM4.*)
       *(.bss.$NCACHE_REGION.*)
       . = ALIGN (. != 0 ? 4 : 1) ; /* avoid empty segment */
       PROVIDE(__end_bss_RAM4 = .) ;
       PROVIDE(__end_bss_NCACHE_REGION = .) ;
    } > NCACHE_REGION AT> NCACHE_REGION

    /* BSS section for SRAM_OC_ECC1 */
    .bss_RAM5 : ALIGN(4)
    {
       PROVIDE(__start_bss_RAM5 = .) ;
       PROVIDE(__start_bss_SRAM_OC_ECC1 = .) ;
       *(.bss.$RAM5)
       *(.bss.$SRAM_OC_ECC1)
       *(.bss.$RAM5.*)
       *(.bss.$SRAM_OC_ECC1.*)
       . = ALIGN (. != 0 ? 4 : 1) ; /* avoid empty segment */
       PROVIDE(__end_bss_RAM5 = .) ;
       PROVIDE(__end_bss_SRAM_OC_ECC1 = .) ;
    } > SRAM_OC_ECC1 AT> SRAM_OC_ECC1

    /* BSS section for SRAM_OC_ECC2 */
    .bss_RAM6 : ALIGN(4)
    {
       PROVIDE(__start_bss_RAM6 = .) ;
       PROVIDE(__start_bss_SRAM_OC_ECC2 = .) ;
       *(.bss.$RAM6)
       *(.bss.$SRAM_OC_ECC2)
       *(.bss.$RAM6.*)
       *(.bss.$SRAM_OC_ECC2.*)
       . = ALIGN (. != 0 ? 4 : 1) ; /* avoid empty segment */
       PROVIDE(__end_bss_RAM6 = .) ;
       PROVIDE(__end_bss_SRAM_OC_ECC2 = .) ;
    } > SRAM_OC_ECC2 AT> SRAM_OC_ECC2

    /* BSS section for RAM_CONVERSA_CONTROL_BLOCK */
    .bss_RAM7 : ALIGN(4)
    {
       PROVIDE(__start_bss_RAM7 = .) ;
       PROVIDE(__start_bss_RAM_CONVERSA_CONTROL_BLOCK = .) ;
       *(.bss.$RAM7)
       *(.bss.$RAM_CONVERSA_CONTROL_BLOCK)
       *(.bss.$RAM7.*)
       *(.bss.$RAM_CONVERSA_CONTROL_BLOCK.*)
       . = ALIGN (. != 0 ? 4 : 1) ; /* avoid empty segment */
       PROVIDE(__end_bss_RAM7 = .) ;
       PROVIDE(__end_bss_RAM_CONVERSA_CONTROL_BLOCK = .) ;
    } > RAM_CONVERSA_CONTROL_BLOCK AT> RAM_CONVERSA_CONTROL_BLOCK

    /* MAIN BSS SECTION */
    .bss : ALIGN(4)
    {
        _bss = .;
        PROVIDE(__start_bss_RAM2 = .) ;
        PROVIDE(__start_bss_SRAM_DTC = .) ;
       *(NonCacheable)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4) ;
        _ebss = .;
        PROVIDE(__end_bss_RAM2 = .) ;
        PROVIDE(__end_bss_SRAM_DTC = .) ;
        PROVIDE(end = .);
    } > SRAM_DTC AT> SRAM_DTC

    /* NOINIT section for SRAM_ITC */
    .noinit_RAM (NOLOAD) : ALIGN(4)
    {
       PROVIDE(__start_noinit_RAM = .) ;
       PROVIDE(__start_noinit_SRAM_ITC = .) ;
       *(.noinit.$RAM)
       *(.noinit.$SRAM_ITC)
       *(.noinit.$RAM.*)
       *(.noinit.$SRAM_ITC.*)
       . = ALIGN(4) ;
       PROVIDE(__end_noinit_RAM = .) ;
       PROVIDE(__end_noinit_SRAM_ITC = .) ;
    } > SRAM_ITC AT> SRAM_ITC

    /* NOINIT section for SRAM_OC1 */
    .noinit_RAM3 (NOLOAD) : ALIGN(4)
    {
       PROVIDE(__start_noinit_RAM3 = .) ;
       PROVIDE(__start_noinit_SRAM_OC1 = .) ;
       *(.noinit.$RAM3)
       *(.noinit.$SRAM_OC1)
       *(.noinit.$RAM3.*)
       *(.noinit.$SRAM_OC1.*)
       . = ALIGN(4) ;
       PROVIDE(__end_noinit_RAM3 = .) ;
       PROVIDE(__end_noinit_SRAM_OC1 = .) ;
    } > SRAM_OC1 AT> SRAM_OC1

    /* NOINIT section for NCACHE_REGION */
    .noinit_RAM4 (NOLOAD) : ALIGN(4)
    {
       PROVIDE(__start_noinit_RAM4 = .) ;
       PROVIDE(__start_noinit_NCACHE_REGION = .) ;
       *(.noinit.$RAM4)
       *(.noinit.$NCACHE_REGION)
       *(.noinit.$RAM4.*)
       *(.noinit.$NCACHE_REGION.*)
       . = ALIGN(4) ;
       PROVIDE(__end_noinit_RAM4 = .) ;
       PROVIDE(__end_noinit_NCACHE_REGION = .) ;
    } > NCACHE_REGION AT> NCACHE_REGION

    /* NOINIT section for SRAM_OC_ECC1 */
    .noinit_RAM5 (NOLOAD) : ALIGN(4)
    {
       PROVIDE(__start_noinit_RAM5 = .) ;
       PROVIDE(__start_noinit_SRAM_OC_ECC1 = .) ;
       *(.noinit.$RAM5)
       *(.noinit.$SRAM_OC_ECC1)
       *(.noinit.$RAM5.*)
       *(.noinit.$SRAM_OC_ECC1.*)
       . = ALIGN(4) ;
       PROVIDE(__end_noinit_RAM5 = .) ;
       PROVIDE(__end_noinit_SRAM_OC_ECC1 = .) ;
    } > SRAM_OC_ECC1 AT> SRAM_OC_ECC1

    /* NOINIT section for SRAM_OC_ECC2 */
    .noinit_RAM6 (NOLOAD) : ALIGN(4)
    {
       PROVIDE(__start_noinit_RAM6 = .) ;
       PROVIDE(__start_noinit_SRAM_OC_ECC2 = .) ;
       *(.noinit.$RAM6)
       *(.noinit.$SRAM_OC_ECC2)
       *(.noinit.$RAM6.*)
       *(.noinit.$SRAM_OC_ECC2.*)
       . = ALIGN(4) ;
       PROVIDE(__end_noinit_RAM6 = .) ;
       PROVIDE(__end_noinit_SRAM_OC_ECC2 = .) ;
    } > SRAM_OC_ECC2 AT> SRAM_OC_ECC2

    /* NOINIT section for RAM_CONVERSA_CONTROL_BLOCK */
    .noinit_RAM7 (NOLOAD) : ALIGN(4)
    {
       PROVIDE(__start_noinit_RAM7 = .) ;
       PROVIDE(__start_noinit_RAM_CONVERSA_CONTROL_BLOCK = .) ;
       *(.noinit.$RAM7)
       *(.noinit.$RAM_CONVERSA_CONTROL_BLOCK)
       *(.noinit.$RAM7.*)
       *(.noinit.$RAM_CONVERSA_CONTROL_BLOCK.*)
       . = ALIGN(4) ;
       PROVIDE(__end_noinit_RAM7 = .) ;
       PROVIDE(__end_noinit_RAM_CONVERSA_CONTROL_BLOCK = .) ;
    } > RAM_CONVERSA_CONTROL_BLOCK AT> RAM_CONVERSA_CONTROL_BLOCK

    /* DEFAULT NOINIT SECTION */
    .noinit (NOLOAD): ALIGN(4)
    {
        _noinit = .;
        PROVIDE(__start_noinit_RAM2 = .) ;
        PROVIDE(__start_noinit_SRAM_DTC = .) ;
        *(.noinit*)
         . = ALIGN(4) ;
        _end_noinit = .;
       PROVIDE(__end_noinit_RAM2 = .) ;
       PROVIDE(__end_noinit_SRAM_DTC = .) ;        
    } > SRAM_DTC AT> SRAM_DTC

    /*
     * for exception handling/unwind - some Newlib functions (in common
     * with C++ and STDC++) use this.
     */
    .ARM.extab : ALIGN(4)
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > BOARD_FLASH

    .ARM.exidx : ALIGN(4)
    {
        __exidx_start = .;
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        __exidx_end = .;
    } > BOARD_FLASH

    /* Reserve and place Heap within memory map */
    _HeapSize = 0x2000;
    .heap :  ALIGN(4)
    {
        _pvHeapStart = .;
        . += _HeapSize;
        . = ALIGN(4);
        _pvHeapLimit = .;
    } > SRAM_ITC

     _StackSize = 0x2000;
     /* Reserve space in memory for Stack */
    .heap2stackfill  :
    {
        . += _StackSize;
    } > SRAM_ITC
    /* Locate actual Stack in memory map */
    .stack ORIGIN(SRAM_ITC) + LENGTH(SRAM_ITC) - _StackSize - 0:  ALIGN(4)
    {
        _vStackBase = .;
        . = ALIGN(4);
        _vStackTop = . + _StackSize;
    } > SRAM_ITC

    /* Provide basic symbols giving location and size of main text
     * block, including initial values of RW data sections. Note that
     * these will need extending to give a complete picture with
     * complex images (e.g multiple Flash banks).
     */
    _image_start = LOADADDR(.text);
    _image_end = LOADADDR(.data) + SIZEOF(.data);
    _image_size = _image_end - _image_start;
    /* Provide symbols for MIMXRT1170 parts for boot header generation code
     * to set image to be plain load image or XIP.
     * Config : Plain load image = false
     */
    _boot_loadaddr = ORIGIN(BOARD_FLASH);
    _boot_size = LENGTH(BOARD_FLASH);
}