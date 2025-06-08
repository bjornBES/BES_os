#include <arch/i686/irq.h>
#include "drivers/VGA/vga.h"
#include "arch/i686/io.h"
#include "arch/i686/pit.h"

#include <hal/hal.h>

#include <boot/bootparams.h>

#include <libcob.h>
#include <debug.h>
#include "stdio.h"
#include "malloc.h"
#include "memory.h"
#include "string.h"
#include "memory_allocator.h"
#include "ctype.h"
#include "time.h"
#include "unistd.h"
#include "callInt.h"
#include "CobolCalls.h"

#include "syscall/systemcall.h"

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
        if (c == '\b')
        {
            if (bufferIndex > 0)
            {
                int x = 0;
                int y = 0;
                VGA_getcursor(&x, &y);
                log_debug("MAIN", "backspace %u (X,Y): (%u, %u)", bufferIndex, x, y);
                bufferIndex--;
                x--;
                VGA_setcursor(x, y);
                printf(" ");
                buffer[bufferIndex] = 0;
                VGA_setcursor(x, y);
            }
        }
        else
        {
            buffer[bufferIndex] = c;
            bufferIndex++;
            printf("%c", c);
            // log_debug("MAIN", "char: %c - %u 0x%X", c, (uint8_t)c, (uint8_t)c);
        }
    }
}

bool cmpCommand(char *command, char *buffer)
{
    while (*command && *buffer)
    {
        if (!(*command == *buffer || toupper(*command) == *buffer))
        {
            return false;
        }
        command++;
        buffer++;
    }
    return true;
}

void Update()
{
    Page bufferPage;
    Page commandPage;
    char *inputBuffer = (char *)malloc(256, &bufferPage);
    char *command = (char *)malloc(64, &commandPage);
    char* cmdPath = (char *)malloc(MAX_PATH_SIZE, &commandPage);
    strcpy(cmdPath, "/ata0");
    uint32_t bufferSize = 4096;
    void *buffer = malloc(4096, &bufferPage);
    VGA_clrscr();
    printf("BES OS v0.1\n");
    printf("This operating system is under construction.\n\n");
    while (true)
    {
        printf("%s$", cmdPath);
        memset(inputBuffer, 0, 256);
        ReadLine(inputBuffer);

        int count = strcount(inputBuffer, ' ');
        char *loc = strtok(inputBuffer, " ");
        char *argv[count + 1];

        putc('\n');

        log_debug("MAIN", "loc = %s, count = %u", loc, count);
        int argc = 0;

        for (size_t i = 0; i < count + 1; i++)
        {
            if (loc == NULL)
            {
                log_debug("MAIN", "continue loc = %s", loc);
                continue;
            }
            argc++;
            argv[i] = loc;
            log_debug("MAIN", "%u: argv[i] = %s", i, argv[i]);
            if (i == 0)
            {
                command = memcpy(command, loc, sizeof(loc) + 1);
            }
            loc = strtok(NULL, " ");
        }

        if (argv[0][0] == '\0')
        {
            continue;
        }

        log_debug("MAIN", "count = %u", count);

        // Command layout
        // device/bin/Command.cod [args]

        if (cmpCommand("clear", argv[0]) == true)
        {
            VGA_clrscr();
        }
        if (cmpCommand("call", argv[0]) == true)
        {
            MainKernelInCobol();
        }
        if (cmpCommand("int", argv[0]) == true)
        {
            if (count < 1)
            {
                printf("Usage: int <interrupt>\n");
            }
            int interrupt = 0;
            atoi(argv[1], &interrupt);
            IntRegisters regs;
            initregs(&regs);
            regs.eax = 0x55AA55AA; // just a test value
            regs.ebx = 0x12345678; // just a test value
            CallInt(interrupt, &regs);
        }
        if (cmpCommand("c-d", argv[0]) == true)
        {
            if (count < 1)
            {
                printf("Usage: change-device <device>\n");
            }
            char* path = argv[1];
            if (path[0] != '/')
            {
                printf("Path must be absolute!\n");
                continue;
            }
            strcpy(cmdPath, path);
            log_debug("MAIN", "cmdPath = %s", cmdPath);
            continue;
        }
        if (cmpCommand("read-file", argv[0]) == true)
        {
            if (count < 1)
            {
                printf("Usage: read <file>\n");
            }
            char* name = argv[1];
            char* path = (char *)malloc(MAX_PATH_SIZE, &commandPage);
            strcat(path, cmdPath);
            strcat(path, "/");
            strcat(path, name);
            fd_t file = VFS_Open(path);
            VFS_Read(file, buffer, bufferSize);
            VFS_Close(file);
            free(path, &commandPage);
            continue;
        }
        if (cmpCommand("read", argv[0]) == true)
        {
            if (count < 2)
            {
                printf("Usage: read <lbs> <count>\n");
            }
            uint32_t lbs = 0;
            uint32_t sectorcount = 0;
            atoi(argv[1], (int *)&lbs);
            atoi(argv[2], (int *)&sectorcount);
            if (bufferSize < sectorcount * 512)
            {
                log_debug("MAIN", "reallocating buffer %u", sectorcount * 512);
                buffer = realloc(buffer, sectorcount * 512, &bufferPage);
                bufferSize = sectorcount * 512;
                memset(buffer, 0, bufferSize);
            }
            ATA_read(buffer, lbs, sectorcount, GetDeviceUsingId(32));
        }
        if (cmpCommand("hex", argv[0]) == true)
        {
            if (count < 1)
            {
                printf("Usage: hex <file dis>\n");
            }
            fd_t fd = VFS_FD_STDOUT;
            atoi(argv[1], &fd);
            uint8_t *u8Buffer = (uint8_t *)buffer;
            for (uint16_t i = 0; i < bufferSize; i++)
            {
                uint8_t word = u8Buffer[i];

                if (i % 16 == 0)
                {
                    fprintf(fd, "\n%04X\t", i);
                }
                fprintf(fd, "%02X,", word);
            }
        }
        if (cmpCommand("info", argv[0]) == true)
        {
            if (count < 1)
            {
                printf("Usage: hex <file dis>\n");
            }
            fd_t fd = VFS_FD_STDOUT;
            atoi(argv[1], &fd);
            fprintf(fd, "\nBufferSize = %u\n", bufferSize);
        }
        if (cmpCommand("mount", argv[0]) == true)
        {
            if (count < 2)
            {
                printf("Usage: mount <device> <loc>\n");
            }
            int device = 0;
            atoi(argv[1], &device);
            char *loc = argv[2];

            device_t *dev = GetDeviceUsingId(device);
            if (dev == NULL)
            {
                printf("Device %u not found!\n", device);
                continue;
            }

            if (MountDevice(dev, loc))
            {
                printf("Mounted %s on %s\n", dev->name, loc);
                log_info("MAIN", "Mounted %s on %s", dev->name, loc);
            }
            else
            {
                printf("Failed to mount %s on %s\n", dev->name, loc);
                log_err("MAIN", "Failed to mount %s on %s", dev->name, loc);
            }
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
    _init();
    HAL_Initialize();
    initSystemCall();
    i686_ISR_RegisterHandler(2, debug);
    mm_init();

    pit_init();
    keyboard_init();
    vga_initialize();
    printf("MAIN: VGA_init() done\n");
    printf("MAIN: KernelInits() done\n");
}

void KernelStart(BootParams *bootParams)
{
    CopyParams(bootParams);
    pre_vga_initialize(params, &params->VESA);
    
    KernelInits();
    printf("KernelStart\n");

    PrintMemoryRegions();

    VGA_clrscr();
    pci_init(params->PCI.PCIHWCharacteristics);
    printStatus();

    pciScan();
    printStatus();
    log_info("MAIN", "pciScan done");
    VGA_clrscr();
    printf("12345678901234567890123456789012345678901234567890123456789012345678901234567890\n");
    VGA_setcursor(0, 2);
    printf("00000000011111111112222222222333333333344444444445555555555666666666677777777778\n");
    
    
    VGA_setcursor(0, 6);
    printf("1234567890123456789012345678901234567890\n");
    printf("0000000001111111111222222222233333333334\n");
    VGA_clrscr();
    // printf("ABC");
    
    VFS_init();
    log_info("MAIN", "VFS done");
    ATA_init();
    log_info("MAIN", "ATA done");
    PressAnyKeyLoop();

    if (ide_devices_count > 0)
    {
        VGA_clrscr();
        PrintDeviceOut();
        device_t *dev = GetDevice(ata_Device);
        printStatus();
        Page bufferPage;
        if (!MountDevice(dev, "/ata0"))
        {
            printf("Unable to mount /ata0 on %s (%d)!\n", dev->name, dev->id);
            log_crit("MAIN", "Unable to mount /ata0 on %s (%d)!\n", dev->name, dev->id);
            for (;;)
            {
                ;
            }
        }

        printStatus();
        ASM_INT2();
        uint8_t *buffer = (uint8_t *)malloc(512, &bufferPage);
        VGA_clrscr();

        fd_t file = VFS_Open("/ata0/test.txt");
        vfs_node_t *node = VFS_GetNode(file);
        if (node == NULL)
        {
            printf("File not found or invalid file descriptor!\n");
            log_err("MAIN", "File not found or invalid file descriptor!");
            free(buffer, &bufferPage);
            PressAnyKeyLoop();
            return;
        }
        VFS_Read(file, buffer, 512);

        VGA_clrscr();

        printf("Data from test.txt\n");
        printf("%s", buffer);
        for (uint16_t i = 0; i < 512; i++)
        {
            uint8_t word = buffer[(i)];
            if (i % 16 == 0)
            {
                fprintf(VFS_FD_DEBUG, "\n%X\t", i);
            }
            fprintf(VFS_FD_DEBUG, "%X,", word);
        }
        
        ASM_INT2();
        VFS_Close(file);
        free(buffer, &bufferPage);
    }

    PressAnyKeyLoop();
    VGA_clrscr();

    printStatus();
    VGA_clrscr();

    log_info("Main", "This is an info msg!");
    log_warn("Main", "This is a warning msg!");
    log_err("Main", "This is an error msg!");
    log_crit("Main", "This is a critical msg!");

    cob_init(0, NULL);

    Update();

    // i686_IRQ_RegisterHandler(0, timer);

    // crash_me();

    // end:
    for (;;)
        ;
}
