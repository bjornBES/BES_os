#include <arch/i686/irq.h>
#include "arch/i686/vga_text.h"
#include "arch/i686/timer.h"
#include "arch/i686/io.h"

#include <hal/hal.h>

#include <boot/bootparams.h>

#include <debug.h>
#include "stdio.h"
#include "malloc.h"
#include "memory.h"
#include "string.h"
#include "memory_allocator.h"

#include "fs/devfs/devfs.h"
#include "fs/disk.h"

#include "drivers/ATA/ATA.h"
#include "drivers/pci/pci.h"
#include "drivers/Keyboard/keyboard.h"
#include "drivers/ide/ide_controller.h"
#include "drivers/ahci/ahci.h"

#include "printfDriver/printf.h"

#define MODULE "MAIN"

extern void _init();
extern char stack_bottom;
extern char stack_top;
extern char bootParamsAddr; // from my linker script

void crash_me();

void debug(Registers *regs)
{
    char *DebugMODULE = "Debug";
    log_debug(DebugMODULE, "Debug %d Debug", regs->interrupt);

    log_debug(DebugMODULE, "  eax=%x  ebx=%x  ecx=%x  edx=%x  esi=%x  edi=%x",
              regs->eax, regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi);

    log_debug(DebugMODULE, "  esp=%x  ebp=%x  eip=%x  eflags=%x  cs=%x  ds=%x  ss=%x",
              regs->esp, regs->ebp, regs->eip, regs->eflags, regs->cs, regs->ds, regs->ss);

    log_debug(DebugMODULE, "  interrupt=%x  errorcode=%x", regs->interrupt, regs->error);
}

void ReadLine(char *buffer)
{
    uint8_t bufferIndex = 0;
    while (true)
    {
        char c = KeyboardGetKey();
        if (c == 0)
        {
            continue;
        }
        if (c == '\n')
        {
            return;
        }
        else
        {
            buffer[bufferIndex] = c;
            bufferIndex++;
            printf("%c", c);
            log_debug("MAIN", "char: %c - %u 0x%X", c, (uint8_t)c, (uint8_t)c);
        }
    }
}

void Update()
{
    Page bufferPage;
    Page commandPage;
    char *buffer = (char *)malloc(256, &bufferPage);
    char *command = (char *)malloc(64, &commandPage);
    VGA_clrscr();
    printf("BES OS v0.1\n");
    printf("This operating system is under construction.\n\n");
    while (true)
    {
        printf("$");
        memset(buffer, 0, 256);
        ReadLine(buffer);

        int count = strcount(buffer, ' ');
        char *loc = strtok(buffer, " ");
        char *args[count + 1];

        putc('\n');

        log_debug("MAIN", "loc = %s", loc);

        for (size_t i = 0; i < count + 1; i++)
        {
            if (strcmp(loc, ""))
            {
                continue;
            }
            args[i] = loc;
            log_debug("MAIN", "1: args[i] = %s", args[i]);
            log_debug("MAIN", "loc = %s size = %u", loc, sizeof(loc));
            if (i == 0)
            {
                memcpy(command, loc, sizeof(loc) + 1);
            }
            loc = strtok(NULL, " ");
        }

        if (memcmp(command, "clear", sizeof(command) + 1) == false)
        {
            VGA_clrscr();
        }
    }

    free(command, &commandPage);
    free(buffer, &bufferPage);
}

BootParams *params;

void CopyParams(BootParams *src)
{
    params = (BootParams *)&bootParamsAddr;
    uint16_t offset = 0;

    params->BootDevice = src->BootDevice;
    offset += sizeof(src->BootDevice);

    params->Memory.RegionCount = src->Memory.RegionCount;
    offset += sizeof(src->Memory.RegionCount);

    params->Memory.Regions = NULL;

    if (src->Memory.RegionCount > 0)
    {
        params->Memory.Regions = (MemoryRegion *)((char *)&bootParamsAddr + offset);
        memcpy(params->Memory.Regions, src->Memory.Regions, src->Memory.RegionCount * sizeof(MemoryRegion));
        offset += src->Memory.RegionCount * sizeof(MemoryRegion);
    }

    params->VESA.SupportVESA = src->VESA.SupportVESA;
    offset += sizeof(src->VESA.SupportVESA);

    params->VESA.VESACount = src->VESA.VESACount;
    offset += sizeof(src->VESA.VESACount);

    params->VESA.entries = NULL;
    if (src->VESA.VESACount > 0)
    {
        params->VESA.entries = (vesa_mode_info_t *)((char *)&bootParamsAddr + offset);
        memcpy(params->VESA.entries, src->VESA.entries, src->VESA.VESACount * sizeof(vesa_mode_info_t));
        offset += src->VESA.VESACount * sizeof(vesa_mode_info_t);
    }
    if (src->PCI.IsInstalled == true)
    {
        memcpy((char *)&bootParamsAddr + offset, &src->PCI, sizeof(PCIInfo));
        params->PCI = *(PCIInfo *)((char *)&bootParamsAddr + offset);
        offset += sizeof(PCIInfo);
    }
}

void PrintMemoryRegions()
{
    printf("Boot device: %x Memory region count: %d\n", params->BootDevice, params->Memory.RegionCount);
    for (int i = 0; i < params->Memory.RegionCount; i += 2)
    {
        printf("%u: 0x%llx 0x%llx %x | %u: 0x%llx 0x%llx %x\n",
               i,
               params->Memory.Regions[i].Begin,
               params->Memory.Regions[i].Length,
               params->Memory.Regions[i].Type,
               i + 1,
               params->Memory.Regions[i + 1].Begin,
               params->Memory.Regions[i + 1].Length,
               params->Memory.Regions[i + 1].Type);
    }
}

void KernelInits()
{
    log_crit("MAIN", "KernelInits()");
    _init();
    HAL_Initialize();
    log_crit("MAIN", "HAL_Initialize() done");
    mm_init();
    log_crit("MAIN", "mm_init() done");
    
    keyboard_init();
    log_crit("MAIN", "Done keyboard_init()");
    VGA_init(&params->VESA);
    log_crit("MAIN", "VGA_init() done");
    pit_init();
    log_crit("MAIN", "pit_init() done");
    log_crit("MAIN", "KernelInits() done");
}

void printHexFormat(uint8_t *buffer, uint32_t size)
{
}

void KernelStart(BootParams *bootParams)
{
    CopyParams(bootParams);

    KernelInits();

    while (true)
    {
        ;
    }
    

    PrintMemoryRegions();

    i686_ISR_RegisterHandler(2, debug);

    VGA_clrscr();
    pci_init(params->PCI.PCIHWCharacteristics);
    
    PressAnyKeyLoop();
    printStatus();
    
    PressAnyKeyLoop();
    pciScan();
    printStatus();
    log_err("MAIN", "pciScan done");
    
    VFS_init();
    log_err("MAIN", "VFS done");
    // ATA_init();
    log_err("MAIN", "ATA done");

    PressAnyKeyLoop();

    /*
    */
    uint8_t dataBuffer[512] = {0};
    ahci_read_sectors(dataBuffer, 0, 1, GetDevice(ata_Device));
    log_debug("MAIN", "ATA read done");
    for (uint16_t i = 0; i < 512; i++)
    {
        uint8_t word = dataBuffer[(i)];
        if (i % 16 == 0)
        {
            fprintf(VFS_FD_DEBUG, "\n%X\t", i);
            fprintf(VFS_FD_STDOUT, "\n%X\t", i);
        }
        fprintf(VFS_FD_DEBUG, "%X,", word);
        fprintf(VFS_FD_STDOUT, "%X,", word);
    }
    
    sleepSec(10);
    printf("sleep done");
    PressAnyKeyLoop();

    VGA_clrscr();
    PrintDeviceOut();
    device_t *dev = GetDevice(ata_Device);
    printf("tring to mount %s on / (%d)!\n", dev->name, dev->id);
    printStatus();
    // dev->name;
    Page bufferPage;
    if (tryMountFS(dev, "/"))
    {
        log_debug("MAIN", "done mounting");

        PrintDeviceOut();
        PressAnyKeyLoop();

        printStatus();
        log_debug("MAIN", "in if stmt");
        ASM_INT2();
        uint8_t *buffer = (uint8_t *)malloc(512, &bufferPage);
        log_debug("MAIN", "after malloc");
        VGA_clrscr();
        VFS_Read("/test.txt", buffer);
        PressAnyKeyLoop();

        VGA_clrscr();

        printf("Data from test.txt");
        for (uint16_t i = 0; i < 512; i++)
        {
            uint8_t word = buffer[(i)];
            if (i % 16 == 0)
            {
                fprintf(VFS_FD_DEBUG, "\n%X\t", i);
                fprintf(VFS_FD_STDOUT, "\n%X\t", i);
            }
            fprintf(VFS_FD_DEBUG, "%X,", word);
            fprintf(VFS_FD_STDOUT, "%X,", word);
        }

        ASM_INT2();
        free(buffer, &bufferPage);
    }
    else
    {
        printf("Unable to mount / on %s (%d)!\n", dev->name, dev->id);
    }

    PressAnyKeyLoop();

    VGA_clrscr();

    printStatus();
    PressAnyKeyLoop();
    VGA_clrscr();

    PressAnyKeyLoop();

    log_info("Main", "This is an info msg!");
    log_warn("Main", "This is a warning msg!");
    log_err("Main", "This is an error msg!");
    log_crit("Main", "This is a critical msg!");

    Update();

    // i686_IRQ_RegisterHandler(0, timer);

    // crash_me();

    // end:
    for (;;)
        ;
}
