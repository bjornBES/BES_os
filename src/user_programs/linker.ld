ENTRY(start)
OUTPUT_FORMAT("binary")
phys = 0x00010000;

SECTIONS
{
    . = phys;
    .text               : { __text_start = .;       *(.text)    }
    .data               : { __data_start = .;       *(.data)    }
    .rodata             : { __rodata_start = .;     *(.rodata)  }
    .bss                : 
    { 
        __bss_start = .;        *(.bss)     
    }
}
