/*
******************************************************************************
**
** @file        : LinkerScript.ld
**
** @brief       : Linker script for STM32F407VETx
**
**              FLASH    : 256 KB
**              USER_FLASH: 128 KB
**              RETENTIVE : 64 KB
**              RAM       : 128 KB
**              CCMRAM    : 64 KB
**
******************************************************************************
*/

/* Entry Point */
ENTRY(Reset_Handler)


/* Highest address of the user mode stack */
_estack = ORIGIN(RAM) + LENGTH(RAM);


/* Minimum heap and stack sizes */
_Min_Heap_Size = 0x200;
_Min_Stack_Size = 0x400;


/*
==============================================================================
MEMORY
==============================================================================
*/

MEMORY
{
  /*
   * Main firmware flash
   *
   * 0x08000000
   * +
   * 256 KB
   * =
   * 0x08040000
   */
  FLASH (rx) :
    ORIGIN = 0x08000000,
    LENGTH = 256K


  /*
   * User-program area
   *
   * 0x08040000
   * +
   * 128 KB
   * =
   * 0x08060000
   */
  USER_FLASH (rx) :
    ORIGIN = 0x08040000,
    LENGTH = 128K


  /*
   * Retentive data
   *
   * 0x08060000
   * +
   * 64 KB
   * =
   * 0x08070000
   */
  RETENTIVE (rx) :
    ORIGIN = 0x08060000,
    LENGTH = 64K


  /*
   * STM32F407VETx main SRAM
   *
   * 128 KB
   */
  RAM (xrw) :
    ORIGIN = 0x20000000,
    LENGTH = 128K


  /*
   * STM32F407 CCM RAM
   *
   * 64 KB
   */
  CCMRAM (xrw) :
    ORIGIN = 0x10000000,
    LENGTH = 64K
}


/*
==============================================================================
SECTIONS
==============================================================================
*/

SECTIONS
{

  /*
  ---------------------------------------------------------------------------
  Interrupt Vector Table
  ---------------------------------------------------------------------------
  */

  .isr_vector :
  {
    . = ALIGN(4);

    KEEP(*(.isr_vector))

    . = ALIGN(4);

  } >FLASH


  /*
  ---------------------------------------------------------------------------
  Program Code
  ---------------------------------------------------------------------------
  */

  .text :
  {
    . = ALIGN(4);

    *(.text)
    *(.text*)

    *(.glue_7)
    *(.glue_7t)

    *(.eh_frame)

    KEEP(*(.init))
    KEEP(*(.fini))

    . = ALIGN(4);

    _etext = .;

  } >FLASH


  /*
  ---------------------------------------------------------------------------
  Read-only data
  ---------------------------------------------------------------------------
  */

  .rodata :
  {
    . = ALIGN(4);

    *(.rodata)
    *(.rodata*)

    . = ALIGN(4);

  } >FLASH


  /*
  ---------------------------------------------------------------------------
  ARM exception table
  ---------------------------------------------------------------------------
  */

  .ARM.extab (READONLY) :
  {
    . = ALIGN(4);

    *(.ARM.extab* .gnu.linkonce.armextab.*)

    . = ALIGN(4);

  } >FLASH


  .ARM (READONLY) :
  {
    . = ALIGN(4);

    __exidx_start = .;

    *(.ARM.exidx*)

    __exidx_end = .;

    . = ALIGN(4);

  } >FLASH


  /*
  ---------------------------------------------------------------------------
  Preinit array
  ---------------------------------------------------------------------------
  */

  .preinit_array (READONLY) :
  {
    . = ALIGN(4);

    PROVIDE_HIDDEN(__preinit_array_start = .);

    KEEP(*(.preinit_array*))

    PROVIDE_HIDDEN(__preinit_array_end = .);

    . = ALIGN(4);

  } >FLASH


  /*
  ---------------------------------------------------------------------------
  Init array
  ---------------------------------------------------------------------------
  */

  .init_array (READONLY) :
  {
    . = ALIGN(4);

    PROVIDE_HIDDEN(__init_array_start = .);

    KEEP(*(SORT(.init_array.*)))
    KEEP(*(.init_array*))

    PROVIDE_HIDDEN(__init_array_end = .);

    . = ALIGN(4);

  } >FLASH


  /*
  ---------------------------------------------------------------------------
  Fini array
  ---------------------------------------------------------------------------
  */

  .fini_array (READONLY) :
  {
    . = ALIGN(4);

    PROVIDE_HIDDEN(__fini_array_start = .);

    KEEP(*(SORT(.fini_array.*)))
    KEEP(*(.fini_array*))

    PROVIDE_HIDDEN(__fini_array_end = .);

    . = ALIGN(4);

  } >FLASH


  /*
  ---------------------------------------------------------------------------
  Initialized data
  ---------------------------------------------------------------------------
  */

  _sidata = LOADADDR(.data);

  .data :
  {
    . = ALIGN(4);

    _sdata = .;

    *(.data)
    *(.data*)

    *(.RamFunc)
    *(.RamFunc*)

    . = ALIGN(4);

    _edata = .;

  } >RAM AT>FLASH


  /*
  ---------------------------------------------------------------------------
  CCMRAM
  ---------------------------------------------------------------------------
  */

  _siccmram = LOADADDR(.ccmram);

  .ccmram :
  {
    . = ALIGN(4);

    _sccmram = .;

    *(.ccmram)
    *(.ccmram*)

    . = ALIGN(4);

    _eccmram = .;

  } >CCMRAM AT>FLASH


  /*
  ---------------------------------------------------------------------------
  BSS
  ---------------------------------------------------------------------------
  */

  . = ALIGN(4);

  .bss :
  {
    _sbss = .;

    __bss_start__ = _sbss;

    *(.bss)
    *(.bss*)

    *(COMMON)

    . = ALIGN(4);

    _ebss = .;

    __bss_end__ = _ebss;

  } >RAM


  /*
  ---------------------------------------------------------------------------
  Heap + Stack
  ---------------------------------------------------------------------------
  */

  ._user_heap_stack :
  {
    . = ALIGN(8);

    PROVIDE(end = .);
    PROVIDE(_end = .);

    . = . + _Min_Heap_Size;

    . = . + _Min_Stack_Size;

    . = ALIGN(8);

  } >RAM


  /*
  ---------------------------------------------------------------------------
  USER PROGRAM
  ---------------------------------------------------------------------------
  *
  * These sections are placed in USER_FLASH.
  *
  * USER_FLASH starts at:
  *
  *     0x08040000
  *
  */

  .user_code_OB100 :
  {
    . = ALIGN(4);

    KEEP(*(.user_code_OB100))

    . = ALIGN(4);

  } >USER_FLASH


  .user_code_OB1 :
  {
    . = ALIGN(4);

    KEEP(*(.user_code_OB1))

    . = ALIGN(4);

  } >USER_FLASH


  .user_code_OB30 :
  {
    . = ALIGN(4);

    KEEP(*(.user_code_OB30))

    . = ALIGN(4);

  } >USER_FLASH


  /*
  ---------------------------------------------------------------------------
  RETENTIVE DATA
  ---------------------------------------------------------------------------
  *
  * Stored in the dedicated RETENTIVE Flash region.
  *
  * RETENTIVE starts at:
  *
  *     0x08060000
  *
  */

  .retentive :
  {
    . = ALIGN(4);

    *(.retentive*)

    . = ALIGN(4);

  } >RETENTIVE


  /*
  ---------------------------------------------------------------------------
  Discard unused library sections
  ---------------------------------------------------------------------------
  */

  /DISCARD/ :
  {
    libc.a(*)
    libm.a(*)
    libgcc.a(*)
  }


  /*
  ---------------------------------------------------------------------------
  ARM attributes
  ---------------------------------------------------------------------------
  */

  .ARM.attributes 0 :
  {
    *(.ARM.attributes)
  }

}