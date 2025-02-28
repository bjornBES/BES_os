#include "memory_allocator.h"
#include "debug.h"
#include "memory.h"

#include <string.h>

#define MODULE "ALLOC"

extern void *__end;
uint8_t *heap_start; // Start of heap memory

// Bitmap structure to manage page allocation
size_t total_size;
uint8_t *memory_Start = NULL; // Start of memory
uint8_t *memory_End = NULL;   // End of memory
uint32_t memoryAllocated;
uint32_t pagesAllocated;

PageBitmap *bitmap;
Page *pages;
size_t num_pages;

#define MAX_VALUE (256 * 4096)  // 1MB
#define THRESHOLD_PERCENTAGE 50 // Define a threshold percentage (e.g., 50%)

bool check_length(size_t baseValue, size_t value2)
{
    size_t threshold = (MAX_VALUE * THRESHOLD_PERCENTAGE) / 100; // 50% of max value
    return (baseValue > value2) || (baseValue >= threshold);
}

// Function to initialize the bitmap and linked list system
void mm_init(MemoryInfo *memInfo)
{
    pagesAllocated = 0;
    memoryAllocated = 0;

    MemoryRegion memRegion;
    int32_t biggestIndex = -1;
    for (size_t i = 0; i < memInfo->RegionCount; i++)
    {
        if (memInfo->Regions[i].Type == MEMORY_AVAILABLE)
        {
            if (biggestIndex == -1)
            {
                biggestIndex = i;
                memRegion = memInfo->Regions[i];
            }
            else
            {
                if (check_length(memInfo->Regions[biggestIndex].Length, memInfo->Regions[i].Length))
                {
                    continue;
                }
                else
                {
                    biggestIndex = i;
                    memRegion = memInfo->Regions[i];
                }
            }
        }
    }
    log_debug(MODULE, "%u: MEM: start=0x%llx length=0x%llx type=%x\n", biggestIndex, memRegion.Begin, memRegion.Length, memRegion.Type);
    printf("%u: MEM: start=0x%llx length=0x%llx type=%x\n", biggestIndex, memRegion.Begin, memRegion.Length, memRegion.Type);
    heap_start = (uint8_t*)&__end;
    memInfo->Regions[biggestIndex].Type = MEMORY_RESERVED;

    memory_Start = (uint8_t *)((uint32_t)memRegion.Begin);
    memory_End = memory_Start + memRegion.Length;

    total_size = (size_t)(memory_End - memory_Start);
    num_pages = total_size / PAGE_SIZE; // Calculate number of pages
    // Allocate memory for the bitmap struct
    bitmap = (PageBitmap *)memory_Start; // Point bitmap to the start of the memory region

    // Initialize the bitmap struct
    bitmap->size = num_pages;
    bitmap->bitmap = (uint32_t *)(memory_Start + sizeof(PageBitmap)); // The bitmap starts after the struct

    // Initialize all bits to 0 (free)
    memset(bitmap->bitmap, 0, (num_pages / 32) + 1);

    // Initialize the pages
    log_debug(MODULE, "total_size %p", heap_start);
    log_debug(MODULE, "total_size %X", total_size);
    log_debug(MODULE, "(num_pages / 32) %u", (num_pages / 32));
    log_debug(MODULE, "size of PageBitmap %u", sizeof(PageBitmap));
    log_debug(MODULE, "size of Page %u", sizeof(Page));
    log_debug(MODULE, "size of HeapBlock %u", sizeof(HeapBlock));
    log_debug(MODULE, "num page %u", num_pages);
    pages = (Page *)((uint8_t *)memory_Start + sizeof(PageBitmap) + (num_pages / 32) + 1);
    log_debug(MODULE, "addr pages %p", pages);
    for (size_t i = 0; i < num_pages; i++)
    {
        pages[i].heapBlock = NULL;
        pages[i].allocatedBlocks = 0;
    }
}

void mm_test()
{
    Page *page = allocate_page();
    uint8_t *d1 = (uint8_t *)heap_alloc(page, 8);
    uint8_t *array1 = (uint8_t *)heap_alloc(page, 0x100);
    uint8_t *array2 = (uint8_t *)heap_alloc(page, 0x100);
    uint8_t *array3 = (uint8_t *)heap_alloc(page, 0x100);
    uint8_t *array4 = (uint8_t *)heap_alloc(page, 0x300);

    heap_free(page, array4);
    heap_free(page, d1);
    heap_free(page, array3);
    heap_free(page, array2);
    heap_free(page, array1);
}

// Set a bit in the bitmap
void set_bit(size_t index)
{
    bitmap->bitmap[index / 32] |= (1U << (index % 32));
}

// Clear a bit in the bitmap
void clear_bit(size_t index)
{
    bitmap->bitmap[index / 32] &= ~(1U << (index % 32));
}

// Check if a bit is set (page allocated)
bool is_bit_set(size_t index)
{
    return bitmap->bitmap[index / 32] & (1U << (index % 32));
}

// Allocate a page from the bitmap
Page *allocate_page()
{
    size_t num_pages = bitmap->size;

    for (size_t i = 0; i < num_pages; i++)
    {
        if (!is_bit_set(i))
        {
            set_bit(i);
            pagesAllocated++;

            Page *page = &pages[i]; // Use the pages array to allocate the page
            page->heapBlock = NULL;
            page->allocatedBlocks = 0;
            return page;
        }
    }
    return NULL; // No free pages available
}

// Function to free a page and reset its heap
void free_page(Page *page)
{
    // First, free all blocks inside the heap
    size_t pageIndex = (page - pages); // Get the index of the page in the pages array
    clear_bit(pageIndex);
    pagesAllocated--;
    memoryAllocated -= PAGE_SIZE; // Decrement the total memory allocated
}

// Dump the memory status
void dump_memory_status()
{
    size_t num_pages = bitmap->size;
    size_t allocated_pages = 0;
    size_t used_bytes = 0;

    for (size_t i = 0; i < num_pages; i++)
    {
        if (is_bit_set(i))
        {
            allocated_pages++;

            // Calculate the used bytes for this page
            Page *page = &pages[i];
            HeapBlock *current_block = page->heapBlock;
            while (current_block != NULL)
            {
                used_bytes += current_block->size;
                current_block = current_block->nextBlock;
            }
        }
    }

    size_t free_pages = num_pages - allocated_pages;

    printf("[MEMORY DUMP] Total pages: %u\n", num_pages);
    printf("[MEMORY DUMP] Allocated pages: %u\n", allocated_pages);
    printf("[MEMORY DUMP] Free pages: %u\n", free_pages);
    printf("[MEMORY DUMP] Used bytes in allocated pages: %u\n", used_bytes);
}

// Function to allocate a block of memory from a page's heap
void *heap_alloc(Page *page, size_t size)
{
    if (!page)
        return NULL;

    // Allocate metadata inside the page
    HeapBlock *new_block = (HeapBlock *)((uint8_t *)page + sizeof(Page) + page->allocatedBlocks * sizeof(HeapBlock));

    new_block->size = size;
    new_block->nextBlock = NULL;

    // Allocate the actual data at `heap_start` (outside the page)
    new_block->data = heap_start;

    // Move heap_start forward to prevent overlap
    heap_start += size;

    // Link the new block into the page’s heap block list
    if (!page->heapBlock)
    {
        page->heapBlock = new_block;
    }
    else
    {
        HeapBlock *current = page->heapBlock;
        while (current->nextBlock)
        {
            current = current->nextBlock;
        }
        current->nextBlock = new_block;
    }

    page->allocatedBlocks++;
    return new_block->data; // Return pointer to allocated memory
}

// Free a block from a page's heap
void heap_free(Page *page, void *ptr)
{
    if (!page || !ptr) return;

    HeapBlock *prev = NULL;
    HeapBlock *current = page->heapBlock;

    while (current != NULL)
    {
        if (current->data == ptr)
        {
            if (prev != NULL)
            {
                prev->nextBlock = current->nextBlock;
            }
            else
            {
                page->heapBlock = current->nextBlock;
            }

            page->allocatedBlocks--;
            return;
        }

        prev = current;
        current = current->nextBlock;
    }
}