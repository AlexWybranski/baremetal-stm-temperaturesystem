ENTRY(Reset_Handler)

MEMORY
{
    FLASH(xr): ORIGIN = 0x08000000, LENGTH = 1024K
    SRAM1(xrw): ORIGIN = 0x20000000, LENGTH = 96K
}

_estack = ORIGIN(SRAM1) + LENGTH(SRAM1);

SECTIONS
{
    .text :
    {
        *(.vector_table)
        *(.text)
        *(.text.*)
        *(.init)
        *(.fini)
        *(.rodata)
        *(.rodata.*)
        . = ALIGN(4);
        __text_end__ = .;
    }> FLASH

    .data :
    {
        . = ALIGN(4);
        __data_start__ = .;
        *(.data)
        *(.data.*)
        . = ALIGN(4);
        __data_end__ = .;
    }> SRAM1 AT> FLASH

    _sidata = LOADADDR(.data);

    .bss :
    {
        . = ALIGN(4);
        __bss_start__ = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end__ = .;
        . = ALIGN(4);
        end = .;
        __end__ = end;
    }> SRAM1
    
}