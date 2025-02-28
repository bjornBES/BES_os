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

#include "fs/devfs/devfs.h"
#include "fs/disk.h"

#include "drivers/ATA/ATA.h"
#include "drivers/pci/pci.h"
#include "drivers/Keyboard/keyboard.h"

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
    uint8_t *buffer = (uint8_t *)malloc(512);
    uint32_t count = ATA_read(buffer, LBA, 1, device);
    for (uint16_t i = 0; i < count; i++)
    {
        uint8_t word = buffer[(i)];
        log_debug("MAIN", "%X:%X\t %c,", i, word, (char)word);
    }

    free(buffer);
    free(disk);
    free(info);

    return;
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
    char *buffer = (char *)malloc(256);
    uint8_t bufferIndex = 0;
    VGA_clrscr();
    printf("BES OS v0.1\n");
    printf("This operating system is under construction.\n\n");
    printf("$");
    while (true)
    {
        memset(buffer, 0, 256);
        ReadLine(buffer);

        char *command = (char *)malloc(64);
        int count = strcount(buffer, ' ');
        char *loc = strtok(buffer, " ");
        char *args[count + 1];

        putc('\n');

        log_debug("MAIN", "loc = %s", loc);

        for (size_t i = 0; i < count + 1; i++)
        {
            if (loc == "")
            {
                continue;
            }
            args[i] = loc;
            log_debug("MAIN", "1: args[i] = %s", args[i]);
            if (i == 0)
            {
                strcpy(command, loc);
            }
            loc = strtok(NULL, " ");
        }
    }

    free(buffer);
}

void start(BootParams *bootParams)
{
    VGA_init();
    VGA_clrscr();
    HAL_Initialize();
    mm_init(&bootParams->Memory);
    i686_ISR_RegisterHandler(2, debug);
    // call global constructors
    // device_init();

    i686_EnableMCE();

    ATA_init();
    pit_init();
    pci_init();
    keyboard_init();
    _init();

    while (true)
    {
        if (KeyboardGetKey() != 0)
        {
            break;
        }
        continue;
    }

    VGA_clrscr();

    log_debug("Main", "Boot device: %x", bootParams->BootDevice);
    log_debug("Main", "Memory region count: %d", bootParams->Memory.RegionCount);
    for (int i = 0; i < bootParams->Memory.RegionCount; i++)
    {
        log_debug("Main", "MEM: start=0x%llx length=0x%llx type=%x",
                  bootParams->Memory.Regions[i].Begin,
                  bootParams->Memory.Regions[i].Length,
                  bootParams->Memory.Regions[i].Type);
    }

    printf("Boot device: %x Memory region count: %d\n", bootParams->BootDevice, bootParams->Memory.RegionCount);
    for (int i = 0; i < bootParams->Memory.RegionCount; i += 2)
    {
        printf("%u: 0x%llx 0x%llx %x %u: 0x%llx 0x%llx %x\n", i,
               bootParams->Memory.Regions[i].Begin,
               bootParams->Memory.Regions[i].Length,
               bootParams->Memory.Regions[i].Type,
               i + 1,
               bootParams->Memory.Regions[i + 1].Begin,
               bootParams->Memory.Regions[i + 1].Length,
               bootParams->Memory.Regions[i + 1].Type);
    }
    printStatus();
    /*
    while (true)
    {
        if (KeyboardGetKey() != 0)
        {
            break;
        }
        continue;
    }
    */
    VGA_clrscr();

    // readDisk(bootParams->BootDevice);
    /*
    while (true)
    {
            if (KeyboardGetKey() != 0)
            {
                break;
            }
            continue;
        }
        */

    log_info("Main", "This is an info msg!");
    log_warn("Main", "This is a warning msg!");
    log_err("Main", "This is an error msg!");
    log_crit("Main", "This is a critical msg!");

    Update();

    // i686_IRQ_RegisterHandler(0, timer);

    // crash_me();

end:
    for (;;)
        ;
}
