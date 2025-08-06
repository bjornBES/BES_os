#include <stdint.h>
#include "stdio.h"
#include "x86.h"
#include "disk.h"
#include "fat.h"
#include "memdefs.h"
#include "memory.h"
#include "mbr.h"
#include "stdlib.h"
#include "string.h"
#include "elf.h"
#include "memdetect.h"
#include "vesa.h"
#include "pci.h"
#include <boot/bootparams.h>

uint8_t* KernelLoadBuffer = (uint8_t*)MEMORY_LOAD_KERNEL;
uint8_t* Kernel = (uint8_t*)MEMORY_KERNEL_ADDR;

BootParams g_BootParams;

typedef void (*KernelStart)(BootParams* bootParams);

void __attribute__((cdecl)) start(uint16_t bootDrive, void* partition)
{
    clrscr();
    DISK disk;
    if (!DISK_Initialize(&disk, bootDrive))
    {
        printf("Disk init error\r\n");
        goto end;
    }

    Partition part;
    MBR_DetectPartition(&part, &disk, partition);

    if (!FAT_Initialize(&part))
    {
        printf("FAT init error\r\n");
        goto end;
    }

    // prepare boot params
    g_BootParams.BootDevice = bootDrive;
    Memory_Detect(&g_BootParams.Memory);

    Detect_VESA(&g_BootParams.VESA);

    Detect_PCI(&g_BootParams.PCI);

    // load kernel
    KernelStart kernelEntry;
    if (!ELF_Read(&part, "/boot/kernel.elf", (void**)&kernelEntry))
    {
        printf("ELF read failed, booting halted!\n");
        goto end;
    }
    SetVGAMode(0x30);
    // SetVESAMode(103);
    // g_BootParams.CurrentMode = 0;
    /*
    uint8_t* kernel = (uint8_t*)kernelEntry - 0xb50;
    printf("Kernel address 0x%X", kernel);
    for (uint16_t i = 0; i < 512; i++)
    {
        uint8_t word = kernel[(i)];
        if (i % 16 == 0)
        {
            printf("\n%X\t", i);
        }
        printf("%X,", word);
    }
    
    printf("Jumping to kernel at %p\r\n", kernelEntry);
    */
    // execute kernel
    kernelEntry(&g_BootParams);

end:
    for (;;);
}
