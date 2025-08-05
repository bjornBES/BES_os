
// org code https://github.com/levex/osdev/blob/bba025f8cfced6ad1addc625aaf9dab8fa7aef80/memory/paging.c

#include <defaultInclude.h>
#include <stdio.h>
#include <debug.h>
#include "arch/i686/memory_i686.h"

#define MODULE "paging"

#define PAGE_SIZE 0x1000
#define PAGE_PRESENT 0x1
#define PAGE_WRITE   0x2

static uint32_t* page_directory = 0;
static uint32_t page_dir_loc = 0;
static uint32_t* last_page = 0;

extern char __end; // from linker script
extern char KernelStart; // from linker script

static uint32_t* allocPageTable() {
    uint32_t* pt = last_page;
    last_page = (uint32_t*)((uint32_t)last_page + PAGE_SIZE);

    // Clear the page table
    for (int i = 0; i < 1024; i++)
        pt[i] = 0;

    return pt;
}

void pagingMapVirtToPhys(uint32_t virt, uint32_t phys)
{
    uint16_t id = virt >> 22;

    for (int i = 0; i < 1024; i++)
    {
        last_page[i] = 0 | PAGE_WRITE;
        phys += 4096;
    }

    page_directory[id] = ((uint32_t)last_page) | PAGE_PRESENT | PAGE_WRITE;
    last_page = (uint32_t *)(((uint32_t)last_page) + 4096);
    log_debug(MODULE, "Mapping %x (%d) to %x", virt, id, phys);
}

void pagingMapRange(uint32_t virt_start, uint32_t phys_start, uint32_t size) {
    uint32_t virt_end = virt_start + size;
    log_debug(MODULE, "Mapping range: virt_start=0x%x, phys_start=0x%x, size=0x%x", virt_start, phys_start, size);

    for (uint32_t v = virt_start, p = phys_start; v < virt_end; v += PAGE_SIZE, p += PAGE_SIZE) {
        uint32_t pd_index = v >> 22;
        uint32_t pt_index = (v >> 12) & 0x3FF;

        // Allocate a page table if not present
        if (!(page_directory[pd_index] & PAGE_PRESENT)) {
            uint32_t* new_table = allocPageTable();
            page_directory[pd_index] = ((uint32_t)new_table) | PAGE_PRESENT | PAGE_WRITE;
        }

        uint32_t* page_table = (uint32_t*)(page_directory[pd_index] & 0xFFFFF000);
        page_table[pt_index] = (p & 0xFFFFF000) | PAGE_PRESENT | PAGE_WRITE;
    }
}

void* getPhysAddress(void* virt)
{
    uint32_t virt_addr = (uint32_t)virt;
    uint16_t pdid = virt_addr >> 22;
    uint16_t ptid = (virt_addr >> 12) & 0x3FF; // Get the page table index
    uint32_t page_table = page_directory[pdid] & 0xFFFFF000; // Get the page table address
    uint32_t offset = virt_addr & 0x00000FFF; // Get the offset within the page

    return (void*)(page_table + (ptid * 4096) + offset);
    
}

void pagingEnable()
{
    __asm__ volatile("mov %%eax, %%cr3": :"a"(page_dir_loc));	
    log_debug(MODULE, "Enabling paging with page directory at %x", page_dir_loc);
	__asm__ volatile("mov %cr0, %eax");
    log_debug(MODULE, "CR0 before paging enable");
	__asm__ volatile("orl $0x80000000, %eax");
    log_debug(MODULE, "CR0 after paging enable");
	__asm__ volatile("mov %eax, %cr0");
    log_debug(MODULE, "Paging enabled");
}

void pagingInit()
{
    log_info(MODULE, "Initializing paging...");
    page_directory = (uint32_t*)0x400000;
    page_dir_loc = (uint32_t)page_directory;
    last_page = (uint32_t *)0x404000;
    
    for (int i = 0; i < 1024; i++)
    {
        page_directory[i] = 0 | 2; // Set all entries to present and writable
    }

    // Identity map from 0x00000000 to __end (rounded up to page boundary)
    uint32_t kernel_end = ((uint32_t)&__end);
    log_info(MODULE, "Kernel end address: 0x%08X", kernel_end);
    pagingMapRange(0x00000000, 0x00000000, kernel_end);

    // VGA text buffer
    pagingMapRange(0x000B8000, 0x000B8000, PAGE_SIZE);

    pagingMapVirtToPhys(0x400000, 0x400000); // Map 0x400000 to itself

    // pagingEnable();
    log_info(MODULE, "Paging initialized successfully.");
}