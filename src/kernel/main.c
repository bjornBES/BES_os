#include <stdint.h>
#include "stdio.h"
#include "memory.h"
#include <hal/hal.h>
#include <arch/i686/irq.h>
#include <debug.h>
#include <boot/bootparams.h>
#include "drivers/ATA/ATA.h"
#include "fs/devfs/devfs.h"
#include "drivers/pci/pci.h"
#include "malloc.h"
#include "fs/disk.h"
#include "arch/i686/vga_text.h"
#include "arch/i686/timer.h"
#include "arch/i686/io.h"

extern void _init();

void crash_me();

void timer(Registers *regs)
{
    VGA_clrscr();
    printf("Hello world from timer");
}

void debug(Registers *regs)
{
    char *MODULE = "Debug";
    log_debug(MODULE, "Debug %d Debug", regs->interrupt);

    log_debug(MODULE, "  eax=%x  ebx=%x  ecx=%x  edx=%x  esi=%x  edi=%x",
              regs->eax, regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi);

    log_debug(MODULE, "  esp=%x  ebp=%x  eip=%x  eflags=%x  cs=%x  ds=%x  ss=%x",
              regs->esp, regs->ebp, regs->eip, regs->eflags, regs->cs, regs->ds, regs->ss);

    log_debug(MODULE, "  interrupt=%x  errorcode=%x", regs->interrupt, regs->error);
}

void readDisk(uint16_t device)
{
    ATA_Identify_t *info = (ATA_Identify_t *)malloc(sizeof(ATA_Identify_t));
    ATA_identify(info);
    DISK *disk = (DISK *)malloc(sizeof(disk));
    GetDisk(info, disk);
    uint32_t LBA = 0;
    DISK_CHS2LBA(disk, 0, 0, 0, &LBA);
    printf("Hello world");
    uint8_t *buffer = (uint8_t *)malloc(512);
    uint32_t count = ATA_read(buffer, LBA, 1, device);
    for (uint16_t i = 0; i < count; i++)
    {
        uint8_t word = buffer[(i)];
        // log_debug("MAIN", "%X:%X\t %c,", i, word, (char)word);
    }

    
    free(disk);
    free(info);
    free(buffer);
    return;
}

void start(BootParams *bootParams)
{
    VGA_clrscr();
    mm_init(bootParams->Memory.Regions[0]);
    HAL_Initialize();
    i686_ISR_RegisterHandler(2, debug);
    // call global constructors
    // device_init();
    ATA_init();
    pit_init();
    pci_init();
    _init();
    
    log_debug("Main", "Boot device: %x", bootParams->BootDevice);
    log_debug("Main", "Memory region count: %d", bootParams->Memory.RegionCount);
    for (int i = 0; i < bootParams->Memory.RegionCount; i++)
    {
        log_debug("Main", "MEM: start=0x%llx length=0x%llx type=%x",
                  bootParams->Memory.Regions[i].Begin,
                  bootParams->Memory.Regions[i].Length,
                  bootParams->Memory.Regions[i].Type);
    }

    VGA_clrscr();
    ATA_Identify_t *info = (ATA_Identify_t *)malloc(sizeof(ATA_Identify_t));
    ATA_identify(info);
    DISK *disk = (DISK *)malloc(sizeof(disk));
    GetDisk(info, disk);
    uint32_t LBA = 0;
    DISK_CHS2LBA(disk, 0, 0, 0, &LBA);
    uint8_t *buffer = (uint8_t *)malloc(512);
    uint32_t count = ATA_read(buffer, LBA, 1, bootParams->BootDevice);
    for (uint16_t i = 0; i < count; i++)
    {
        uint8_t word = buffer[(i)];
        // log_debug("MAIN", "%X:%X\t %c,", i, word, (char)word);
    }

    
    free(disk);
    free(info);
    free(buffer);
    
    log_info("Main", "This is an info msg!");
    log_warn("Main", "This is a warning msg!");
    log_err("Main", "This is an error msg!");
    log_crit("Main", "This is a critical msg!");

    printStatus();

    printf("BES OS v0.1\n");
    printf("This operating system is under construction.\n");

    // i686_IRQ_RegisterHandler(0, timer);

    // crash_me();

end:
    for (;;)
        ;
}
