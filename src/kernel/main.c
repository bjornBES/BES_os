#include <arch/i686/irq.h>
#include "drivers/VGA/vga.h"
#include "arch/i686/io.h"
#include "arch/i686/pit.h"

#include <hal/hal.h>

#include <boot/bootparams.h>

#include <libcob.h>
#include <debug.h>
#include "stdio.h"
#include "memory.h"
#include "string.h"
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

#include "shellFile.h"

#include <cpuid.h>

#define MODULE "MAIN"

extern void _init();
extern char __end;
extern char stack_bottom;
extern char stack_top;
extern char bootParamsAddr; // from my linker script

void crash_me();

void debug(Registers *regs)
{
    char *DebugMODULE = "Debug";
    log_debug(DebugMODULE, "Debug %d Debug", regs->interrupt);

    log_debug(DebugMODULE, "  eax=%x  ebx=%x  ecx=%x  edx=%x  esi=%x  edi=%x",
              regs->U32.eax, regs->U32.ebx, regs->U32.ecx, regs->U32.edx, regs->U32.esi, regs->U32.edi);

    log_debug(DebugMODULE, "  esp=%x  ebp=%x  eip=%x  eflags=%x  cs=%x  ds=%x  ss=%x",
              regs->esp, regs->U32.ebp, regs->eip, regs->eflags, regs->cs, regs->ds, regs->ss);

    // log_debug(DebugMODULE, "  es=%x  fs=%x  gs=%x",
    //           regs->es, regs->fs, regs->gs);

    log_debug(DebugMODULE, "  interrupt=%x  errorcode=%x", regs->interrupt, regs->error);
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
#include "arch/i686/idt.h"
#include "arch/i686/gdt.h"
void KernelInits()
{
    _init();
    initSystemCall();
    i686_ISR_RegisterHandler(2, debug);
    
    log_debug("MAIN", "init memory allocator");
    mmInit((uint32_t)(uint32_t*)&__end);
    
    i686_DisableInterrupts();
    pagingInit();
    log_debug("MAIN", "init devices");
    initDevice();
    
    log_debug("MAIN", "init pit");
    pit_init();
    
    log_debug("MAIN", "init keyboard");
    keyboard_init();
    
    log_debug("MAIN", "init vga");
    vga_initialize();
    
    printf("MAIN: VGA_init() done\n");
    printf("MAIN: KernelInits() done\n");
    log_debug("MAIN", "KernelStart");
    
    PrintRegisters();
    
    IDT_Load();

    i686_EnableInterrupts();
}

void KernelStart(BootParams *bootParams)
{
    CopyParams(bootParams);
    
    pre_vga_initialize(params, &params->VESA);

    KernelInits();
    printf("KernelStart\n");

    /*
    {
        PressAnyKeyLoop();
        Page* page = GetPage(0);
        void* data1 = malloc(1, page);
        log_err("MAIN", "== ALLOCATED DATA1 ==");
        mmPrintStatus();
        PressAnyKeyLoop();

        void* data2 = malloc(2, page);
        log_err("MAIN", "== ALLOCATED DATA2 ==");
        mmPrintStatus();
        PressAnyKeyLoop();

        void* data3 = malloc(4096 + 1, page);
        log_err("MAIN", "== ALLOCATED DATA3 ==");
        mmPrintStatus();
        PressAnyKeyLoop();

        free(data2, page);
        mmPrintStatus();
        log_err("MAIN", "== FREE DATA2 ==");
        PressAnyKeyLoop();

        free(data3, page);
        mmPrintStatus();
        log_err("MAIN", "== FREE DATA3 ==");
        PressAnyKeyLoop();

        free(data1, page);
        mmPrintStatus();
        log_err("MAIN", "== FREE DATA1 ==");
        PressAnyKeyLoop();
    }
    */

    PrintMemoryRegions();

    VGA_clrscr();
    _log_debug("Init pci devices");
    pci_init(params->PCI.PCIHWCharacteristics);
    mmPrintStatus();
    
    i686_DisableInterrupts();
    _log_debug("Scan pci devicess");
    pciScan();
    i686_EnableInterrupts();
    PressAnyKeyLoop();
    mmPrintStatus();
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
        log_debug("MAIN", "dev is at %p", dev);
        mmPrintStatus();
        if (!MountDevice(dev, "/ata0"))
        {
            printf("Unable to mount /ata0 on %s (%d)!\n", dev->name, dev->id);
            log_crit("MAIN", "Unable to mount /ata0 on %s (%d)!\n", dev->name, dev->id);
            for (;;)
            {
                ;
            }
        }
    }

    log_info("Main", "This is an info msg!");
    log_warn("Main", "This is a warning msg!");
    log_err("Main", "This is an error msg!");
    log_crit("Main", "This is a critical msg!");

    EnterShell();

    // i686_IRQ_RegisterHandler(0, timer);

    // crash_me();

    // end:
    for (;;)
        ;
}
