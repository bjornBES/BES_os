ENTRY(start)
OUTPUT_FORMAT("binary")
phys = 0x00010000;

SECTIONS
{
    . = phys;
    .text               : { __text_start = .;       *(.text)    __text_end = .; }
    .data               : { __data_start = .;       *(.data)    __data_end = .; }
    .rodata             : { __rodata_start = .;     *(.rodata)  __rodata_end = .; }
    .bss                : 
    { 
        __bss_start = .;        *(.bss)    
        __bss_end = .; 
    }
}
