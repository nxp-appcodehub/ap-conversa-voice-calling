/*
 * GENERATED FILE - DO NOT EDIT
 * Copyright (c) 2008-2013 Code Red Technologies Ltd,
 * Copyright 2015, 2018-2019 NXP
 * (c) NXP Semiconductors 2013-2023
 * Generated linker script file for LPC55S69
 * Created from linkscript.ldt by FMCreateLinkLibraries
 * Using Freemarker v2.3.30
 * MCUXpresso IDE v11.6.0 [Build 8187] [2022-07-13] on Nov 17, 2022, 11:41:58 AM
 */

INCLUDE "ConversaSWPMapping_library.ld"
INCLUDE "ConversaSWPMapping_memory.ld"

ENTRY(ResetISR)

SECTIONS
{
   
    
    /* _core_m33slave text section */
    .text.core_m33slave_text : SUBALIGN(4)
    {
       FILL(0xff)
       __core_m33slave_START__ = .; /* start of slave image */
       KEEP(*(.core_m33slave))
    }> Flash_BootCore1


    /* _core_m33slave data section */
    .text.core_m33slave_data . : SUBALIGN(4)
    {
        FILL(0xff)
        KEEP(*(.core_m33slave.data_*)) KEEP(*(.core_m33slave.data))
        __core_m33slave_END__ = .; /* end of slave image */

            ASSERT(!(__core_m33slave_START__ == __core_m33slave_END__), "No slave code for _core_m33slave");
            ASSERT( (ABSOLUTE(__core_m33slave_START__) == __vectors_start___core_m33slave), "M33SLAVE execute address differs from address provided in source image");
    }
    .text_Flash2 : ALIGN(4)
    {
       FILL(0xff)
        *(.text_Flash2) /* for compatibility with previous releases */
        *(.text_Flash_BootCore1) /* for compatibility with previous releases */
        *(.text.$Flash2)
        *(.text.$Flash_BootCore1)
        *(.text_Flash2.*) /* for compatibility with previous releases */
        *(.text_Flash_BootCore1.*) /* for compatibility with previous releases */
        *(.text.$Flash2.*)
        *(.text.$Flash_BootCore1.*)
        *(.rodata.$Flash2)
        *(.rodata.$Flash_BootCore1)
        *(.rodata.$Flash2.*)
        *(.rodata.$Flash_BootCore1.*)            } > Flash_BootCore1

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
        LONG(LOADADDR(.data));
        LONG(    ADDR(.data));
        LONG(  SIZEOF(.data));
        LONG(LOADADDR(.data_RAM2));
        LONG(    ADDR(.data_RAM2));
        LONG(  SIZEOF(.data_RAM2));
        __data_section_table_end = .;
        __bss_section_table = .;
        LONG(    ADDR(.bss));
        LONG(  SIZEOF(.bss));
        LONG(    ADDR(.bss_RAM2));
        LONG(  SIZEOF(.bss_RAM2));
        __bss_section_table_end = .;
        __section_table_end = . ;
        /* End of Global Section Table */

          *(.after_vectors*)
        /********* Import section CreateCodeInFlash in Flash *********/
	      *(.CreateCodeInFlash)
        /********* Import all .text in Flash  BUT exclude (libcnano + CMSIS + Powerquad + Conversa) .text & .rodata from Flash *********/ 
          *(EXCLUDE_FILE (*libc_nano.a *libarm_ARMv8MMLlfsp_math.a *libRdspConversa.a *atan2f.o *fminf.o *fmaxf.o *fsl_powerquad_basic.o *fsl_powerquad_cmsis.o *fsl_powerquad_data.o *fsl_powerquad_math.o *fsl_powerquad_matrix.o *fsl_powerquad_transform.o *fsl_powerquad_filter.o) .text*)
          *(EXCLUDE_FILE (*libc_nano.a *libarm_ARMv8MMLlfsp_math.a *libRdspConversa.a *atan2f.o *fminf.o *fmaxf.o *fsl_powerquad_basic.o *fsl_powerquad_cmsis.o *fsl_powerquad_data.o *fsl_powerquad_math.o *fsl_powerquad_matrix.o *fsl_powerquad_transform.o *fsl_powerquad_filter.o) .rodata*)
          *(EXCLUDE_FILE (*libc_nano.a *libarm_ARMv8MMLlfsp_math.a *libRdspConversa.a *atan2f.o *fminf.o *fmaxf.o *fsl_powerquad_basic.o *fsl_powerquad_cmsis.o *fsl_powerquad_data.o *fsl_powerquad_math.o *fsl_powerquad_matrix.o *fsl_powerquad_transform.o *fsl_powerquad_filter.o) .rodata)
       . = ALIGN(4);
    } > PROGRAM_FLASH
 
    _etext = .;
        
    /* DATA section for SRAMX */

    .data_RAM2 : ALIGN(4)
    {
        FILL(0xff)
        PROVIDE(__start_data_RAM2 = .) ;
        PROVIDE(__start_data_SRAMX = .) ;
        *(.ramfunc.$RAM2)
        *(.ramfunc.$SRAMX)
        *(.data.$RAM2)
        *(.data.$SRAMX)
        *(.data.$RAM2.*)
        *(.data.$SRAMX.*)
         *(CodeQuickAccessInCodeRam)          
      /********* Conversa .text *********/
        *libRdspConversa.a:*(.text .text* .text.*)
      /********* atan  *********/ 
        *atan2f.o     (.text .text* .text.*) 
        *atan2f.o     (.rodata .rodata.* )      
        *fminf.o     (.text .text* .text.*) 
        *fminf.o     (.rodata .rodata.* )       
        *fmaxf.o     (.text .text* .text.*) 
        *fmaxf.o     (.rodata .rodata.* )      
        . = ALIGN(4) ;
        PROVIDE(__end_data_RAM2 = .) ;
        PROVIDE(__end_data_SRAMX = .) ;
     } > SRAMX AT>PROGRAM_FLASH

    /* MAIN DATA SECTION */
    .uninit_RESERVED (NOLOAD) : ALIGN(4)
    {
        _start_uninit_RESERVED = .;
        KEEP(*(.bss.$RESERVED*))
       . = ALIGN(4) ;
        _end_uninit_RESERVED = .;
    } > Ram0 AT> Ram0

    /* Main DATA section (Ram0) */
    .data : ALIGN(4)
    {
       FILL(0xff)
       _data = . ;
       PROVIDE(__start_data_RAM = .) ;
       PROVIDE(__start_data_Ram0 = .) ;
       *(vtable)
       *(.ramfunc*)
       *(CodeQuickAccessInDataRam)
       KEEP(*(DataQuickAccess))
       *(RamFunction)
       *(.data*)       
       /********* CMSIS LIB (text + rodata + constdata) *********/
         *libarm_ARMv8MMLlfsp_math.a:*(.text .text* .text.* .rodata .rodata.* )        
       /********* libcnano  (text + rodata + constdata) *********/
         *libc_nano.a:*(.text .text* .text.* .rodata .rodata.* )
       /********* Conversa  ( rodata + constdata)       *********/
         *libRdspConversa.a:*(.rodata .rodata.* ) 
       /********* POWERQUAD .text *********/
         *fsl_powerquad_basic.o     (.text .text* .text.*)
         *fsl_powerquad_filter.o    (.text .text* .text.*)
         *fsl_powerquad_cmsis.o     (.text .text* .text.*)
         *fsl_powerquad_data.o      (.text .text* .text.*) 
         *fsl_powerquad_math.o      (.text .text* .text.*)  
         *fsl_powerquad_matrix.o    (.text .text* .text.*)
         *fsl_powerquad_transform.o (.text .text* .text.*) 
       /*********POWERQUAD  ( rodata + constdata)       *********/  
         *fsl_powerquad_basic.o     (.rodata .rodata.* )
         *fsl_powerquad_filter.o    (.rodata .rodata.* )
         *fsl_powerquad_cmsis.o     (.rodata .rodata.* )
         *fsl_powerquad_data.o      (.rodata .rodata.* ) 
         *fsl_powerquad_math.o      (.rodata .rodata.* )  
         *fsl_powerquad_matrix.o    (.rodata .rodata.* )
         *fsl_powerquad_transform.o (.rodata .rodata.* )  
       . = ALIGN(4) ;
       _edata = . ;
       PROVIDE(__end_data_RAM = .) ;
       PROVIDE(__end_data_Ram0 = .) ;
    } > Ram0 AT>PROGRAM_FLASH

    /* BSS section for SRAMX */
    .bss_RAM2 : ALIGN(4)
    {
       PROVIDE(__start_bss_RAM2 = .) ;
       PROVIDE(__start_bss_SRAMX = .) ;
       *(.bss.$RAM2)
       *(.bss.$SRAMX)
       *(.bss.$RAM2.*)
       *(.bss.$SRAMX.*)       
       . = ALIGN (. != 0 ? 4 : 1) ; /* avoid empty segment */
       PROVIDE(__end_bss_RAM2 = .) ;
       PROVIDE(__end_bss_SRAMX = .) ;
    } > SRAMX AT> SRAMX

    /* MAIN BSS SECTION */
    .bss : ALIGN(4)
    {
        _bss = .;
        PROVIDE(__start_bss_RAM = .) ;
        PROVIDE(__start_bss_Ram0 = .) ;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4) ;
        _ebss = .;
        PROVIDE(__end_bss_RAM = .) ;
        PROVIDE(__end_bss_Ram0 = .) ;
        PROVIDE(end = .);
    } > Ram0 AT> Ram0

    /* NOINIT section for SRAMX */
    .noinit_RAM2 (NOLOAD) : ALIGN(4)
    {
       PROVIDE(__start_noinit_RAM2 = .) ;
       PROVIDE(__start_noinit_SRAMX = .) ;
       *(.noinit.$RAM2)
       *(.noinit.$SRAMX)
       *(.noinit.$RAM2.*)
       *(.noinit.$SRAMX.*)
       . = ALIGN(4) ;
       PROVIDE(__end_noinit_RAM2 = .) ;
       PROVIDE(__end_noinit_SRAMX = .) ;
    } > SRAMX AT> SRAMX

    /* DEFAULT NOINIT SECTION */
    .noinit (NOLOAD): ALIGN(4)
    {
        _noinit = .;
        PROVIDE(__start_noinit_RAM = .) ;
        PROVIDE(__start_noinit_Ram0 = .) ;
        *(.noinit*)
         . = ALIGN(4) ;
        _end_noinit = .;
       PROVIDE(__end_noinit_RAM = .) ;
       PROVIDE(__end_noinit_Ram0 = .) ;        
    } > Ram0 AT> Ram0

    /* _core_m33slave extab and exidx sections */
    .text.core_m33slave_ARM_extab . : SUBALIGN(4)
    {
        FILL(0xff)
        KEEP(*(.core_m33slave.ARM.extab))
    }

    .text.core_m33slave_ARM_exidx . : SUBALIGN(4)
    {
        FILL(0xff)
        KEEP(*(.core_m33slave.ARM.exidx))
    }

    /*
     * for exception handling/unwind - some Newlib functions (in common
     * with C++ and STDC++) use this.
     */
    .ARM.extab : ALIGN(4)
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > PROGRAM_FLASH

    .ARM.exidx : ALIGN(4)
    {
        __exidx_start = .;
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        __exidx_end = .;
    } > PROGRAM_FLASH

    /* Reserve and place Heap within memory map */
    /*_HeapSize = 0x1AC30;*/
     _HeapSize = 0x186A0;
    .heap :  ALIGN(4)
    {
        _pvHeapStart = .;
        . += _HeapSize;
        . = ALIGN(4);
        _pvHeapLimit = .;
    } > Ram0

     _StackSize = 0x950;
     /* Reserve space in memory for Stack */
    .heap2stackfill  :
    {
        . += _StackSize;
    } > Ram0
    /* Locate actual Stack in memory map */
    .stack ORIGIN(Ram0) + LENGTH(Ram0) - _StackSize - 0:  ALIGN(4)
    {
        _vStackBase = .;
        . = ALIGN(4);
        _vStackTop = . + _StackSize;
    } > Ram0

    /* ## Create checksum value (used in startup) ## */
    PROVIDE(__valid_user_code_checksum = 0 - 
                                         (_vStackTop 
                                         + (ResetISR + 1) 
                                         + (NMI_Handler + 1) 
                                         + (HardFault_Handler + 1) 
                                         + (( DEFINED(MemManage_Handler) ? MemManage_Handler : 0 ) + 1)   /* MemManage_Handler may not be defined */
                                         + (( DEFINED(BusFault_Handler) ? BusFault_Handler : 0 ) + 1)     /* BusFault_Handler may not be defined */
                                         + (( DEFINED(UsageFault_Handler) ? UsageFault_Handler : 0 ) + 1) /* UsageFault_Handler may not be defined */
                                         ) );

    /* Provide basic symbols giving location and size of main text
     * block, including initial values of RW data sections. Note that
     * these will need extending to give a complete picture with
     * complex images (e.g multiple Flash banks).
     */
    _image_start = LOADADDR(.text);
    _image_end = LOADADDR(.data) + SIZEOF(.data);
    _image_size = _image_end - _image_start;
}