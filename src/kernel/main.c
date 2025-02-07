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

extern void _init();

void crash_me();

void timer(Registers* regs)
{
    printf(".");
}

void readDisk()
{
    ATA_Identify_t* info = (ATA_Identify_t*)malloc(sizeof(ATA_Identify_t));
    ATA_identify(info);
    uint8_t* buffer = (uint8_t*)malloc(512);
    device_t* device = device_get_by_id(32);
    device->read(buffer, )
}

void start(BootParams* bootParams)
{   
    // call global constructors
    VFS_init();
    device_init();
    ata_init();
    pci_init();
    _init();

    HAL_Initialize();

    log_debug("Main", "Boot device: %x", bootParams->BootDevice);
    log_debug("Main", "Memory region count: %d", bootParams->Memory.RegionCount);
    for (int i = 0; i < bootParams->Memory.RegionCount; i++) 
    {
        log_debug("Main", "MEM: start=0x%llx length=0x%llx type=%x", 
            bootParams->Memory.Regions[i].Begin,
            bootParams->Memory.Regions[i].Length,
            bootParams->Memory.Regions[i].Type);
    }


    log_info("Main", "This is an info msg!");
    log_warn("Main", "This is a warning msg!");
    log_err("Main", "This is an error msg!");
    log_crit("Main", "This is a critical msg!");
    printf("BES OS v0.1\n");
    printf("This operating system is under construction.\n");

    //i686_IRQ_RegisterHandler(0, timer);

    //crash_me();

end:
    for (;;);
}
