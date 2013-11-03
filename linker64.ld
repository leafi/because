ENTRY (_start)

OUTPUT_FORMAT (binary)
 
SECTIONS
{
    /* 32bit>64bit loader WOULD load us at 1mb + 64k. */
    /* We're using Pure64 now, which loads us at 1mb! */
    . = 0x00100000;
 
    .text ALIGN(0x1000) :
    {
        *(.text)
    }
 
    .rodata ALIGN(0x1000) :
    {
        *(.rodata*)
    }
 
    .data ALIGN(0x1000) :
    {
        *(.data)
    }
 
    .bss ALIGN(0x1000) :
    {
        *(COMMON)
        *(.bss)
    }
}

