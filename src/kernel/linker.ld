ENTRY(start)
OUTPUT_FORMAT("elf32-i386")
phys = 0x00100000;

SECTIONS
{
    . = phys;
    
    .StartText          : { *(.StartText) }
    .text               : { __text_start = .;       *(.text)    }
    .data               : { __data_start = .;       *(.data)    }
    .rodata             : { __rodata_start = .;     *(.rodata)  }
    .bss                : 
    { 
        __bss_start = .;        *(.bss)     
        isr_stack_bottom = ALIGN(4096);
        . += 16384;  /* 16 KB ISR Stack */
        isr_stack_top = .;
    }
    stack_bottom = .;
    stack_bottom = ALIGN(4096);
    . += 16384;
    stack_top = .;
    stack_top = ALIGN(4096);
    
    bootParamsAddr = .;
    bootParamsAddr = ALIGN(4096);  /* Align bootParamsAddr to the next 4KB boundary */
    . += 4096;
    
    VGAModesAddr = .;
    VGAModesAddr = ALIGN(4096);  /* Align VGAModesAddr to the next 4KB boundary */
    . += 8192;
    
    __end = .;
    __end = ALIGN(4096);  /* Align __end to the next 4KB boundary */
}
