#include "memory_allocator.h"
#include "debug.h"
#include "memory.h"

#include <string.h>
#include <stddef.h>

#include "memory_allocatorconfig.h"

#define MODULE "ALLOC"

extern char __end;
uint8_t *heap_start; // Start of heap memory

HeapBlock *FirstBlock;

// Bitmap structure to manage page allocation
size_t total_size;
uint8_t *memory_Start = NULL; // Start of memory
uint8_t *memory_End = NULL;   // End of memory
uint32_t memoryAllocated;
uint32_t pagesAllocated;

uint32_t variabelsEnd;

PageBitmap *bitmap;
Page *pages;
size_t num_pages;
int32_t pageProsses;
HeapBlock *nextBlock;

#define MAX_VALUE (256 * 4096) * 4 // 4MB
#define PAGE_BLOCKSPACE BLOCKSPREPAGE * sizeof(HeapBlock)

Page *allocatePageWithIndex(int i);

// Function to initialize the bitmap and linked list system
void mm_init()
{
    pageProsses = 0;
    pagesAllocated = 0;
    memoryAllocated = 0;

    memory_Start = (uint8_t *)&__end;

    // Allocate memory for the bitmap struct
    bitmap = (PageBitmap *)memory_Start; // Point bitmap to the start of the memory region

    // Initialize the bitmap struct
    bitmap->bitmap = (uint32_t *)(memory_Start + sizeof(PageBitmap)); // The bitmap starts after the struct

    uint16_t page32 = minPage / 32;
    uint8_t *bitmapEnd = (uint8_t *)((memory_Start + sizeof(PageBitmap)) + minPage / 32);

    memory_End = (uint8_t *)(bitmapEnd + (minPage * sizeof(Page)) - 1);

    total_size = (size_t)((memory_End + 1) - bitmapEnd + 1);
    num_pages = total_size / sizeof(Page); // Calculate number of pages

    bitmap->size = num_pages;
    // Initialize all bits to 0 (free)
    memset(bitmap->bitmap, 0, (num_pages / 32) + 1);

    // Initialize the pages
    pages = (Page *)((uint8_t *)bitmapEnd + 1);
    Page *lastPage = &pages[num_pages - 1];

    variabelsEnd = PGROUNDUP(((uint32_t)&pages[num_pages - 1]));
    FirstBlock = (HeapBlock *)variabelsEnd;
    uint8_t *currentBlock = (uint8_t *)variabelsEnd;
    log_debug(MODULE, "start Block at %p", currentBlock);
    for (size_t i = 0; i < num_pages; i++)
    {
        pages[i].heapBlock = NULL;
        pages[i].allocatedBlocks = 0;
        pages[i].prosses = -1;
        currentBlock += PAGE_BLOCKSPACE;
    }
    log_debug(MODULE, "last block at %p", currentBlock);

    heap_start = (uint8_t *)currentBlock + PAGE_BLOCKSPACE;

    log_info(MODULE, "__end = %p", &__end);
    log_info(MODULE, "variabelsEnd = %p", variabelsEnd);
    log_info(MODULE, "heap start %p", heap_start);
    log_info(MODULE, "memory start: %p, memory_End: %p, memory Size: %p", memory_Start, memory_End, memory_End - memory_Start);
    log_info(MODULE, "total size 0x%X", total_size);
    log_info(MODULE, "num page %u", num_pages);
    log_info(MODULE, "bitmap: [%p-%p] %u", bitmap, bitmapEnd, page32);
    log_info(MODULE, "pages: [%p-%p]", pages, lastPage);
    log_info(MODULE, "blocks: [%p-%p] 0x%x", FirstBlock, currentBlock, (HeapBlock *)currentBlock - FirstBlock);
    log_debug(MODULE, "PAGE_BLOCKSPACE = %u/0x%x", PAGE_BLOCKSPACE, PAGE_BLOCKSPACE);

    // allocating page 0 for general stuff
    allocatePageWithIndex(0);
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

Page *allocatePageWithIndex(int i)
{
    if (!is_bit_set(i))
    {
        // log_debug(MODULE, "found bit %u", i);
        set_bit(i);
        pagesAllocated++;

        Page *page = &pages[i]; // Use the pages array to allocate the page
        page->heapBlock = NULL;
        page->allocatedBlocks = 0;
        page->prosses = pageProsses;
        // log_warn(MODULE, "pageProsses: %u", pageProsses);
        pageProsses++;
        return page;
    }
    return NULL;
}

Page *GetPage(int pageIndex)
{
    return &pages[pageIndex];
}

// Allocate a page from the bitmap
Page *allocate_page()
{
    // log_warn(MODULE, "New page pageProsses: %u, pagesAllocated: %u", pageProsses, pagesAllocated);
    // log_debug(MODULE, "In allocate_page function");
    size_t num_pages = bitmap->size;

    for (size_t i = 0; i < num_pages; i++)
    {
        Page *page = allocatePageWithIndex(i);
        if (page == NULL)
        {
            continue;
        }
        if (page->prosses > num_pages)
        {
            log_crit(MODULE, "page id %u is over %u", page->prosses, num_pages);
        }
        return page;
    }
    return NULL; // No free pages available
}

// Function to free a page and reset its heap
void free_page(Page *page)
{
    // log_debug(MODULE, "in free_page function");
    // log_debug(MODULE, "page addr: %p, page.allocatedBlocks: %u, page.heapBlock: 0x%p", page, page->allocatedBlocks, page->heapBlock);
    // First, free all blocks inside the heap
    size_t pageIndex = (page - pages); // Get the index of the page in the pages array
    if (pageIndex == 0)
    {
        return;
    }
    // log_debug(MODULE, "pageIndex: %u", pageIndex);
    clear_bit(pageIndex);
    page->prosses = -1;
    pagesAllocated--;
    pageProsses--;
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
            if (pages[i].prosses == -1)
            {
                continue;
            }
            allocated_pages++;

            // Calculate the used bytes for this page
            Page *page = &pages[i];
            log_info("MEMORY DUMP", "- pages[%u]: %p heapBlock(%p), allocatedBlocks: %x pageProsses:%u", i, &pages[i], &pages[i].heapBlock, pages[i].allocatedBlocks, pages[i].prosses);
            HeapBlock *current_block = page->heapBlock;
            int count = 0;
            while (current_block != NULL)
            {
                used_bytes += current_block->size;
                log_info("MEMORY DUMP", "  - current_block[%u]: %p size(%p):%u, Next(%p), data(%p)", count, current_block, &(current_block->size), current_block->size, current_block->nextBlock, current_block->data);
                current_block = current_block->nextBlock;
                count++;
            }
        }
    }

    size_t free_pages = num_pages - allocated_pages;

    log_info("MEMORY DUMP", "bitmap: 0x%p, &size: 0x%p, &bitmap addr: 0x%p", bitmap, &bitmap->size, bitmap->bitmap);
    log_info("MEMORY DUMP", "pages: 0x%p", pages);

    log_info("MEMORY DUMP", "Total pages: %u", num_pages);
    log_info("MEMORY DUMP", "Allocated pages: %u", allocated_pages);
    log_info("MEMORY DUMP", "Free pages: %u", free_pages);
    log_info("MEMORY DUMP", "Used bytes in allocated pages: %u", used_bytes);
}

// Function to allocate a block of memory within a given page
void *heap_alloc(Page *page, size_t size)
{
    // Check if the requested size is valid
    // log_debug(MODULE, "page addr: 0x%p, size = %u heap_start = 0x%p", page, size, heap_start);
    if (size == 0)
    {
        log_err(MODULE, "return NULL in heap_alloc");
        return NULL; // Cannot allocate if the size is invalid or exceeds a page size
    }

    if (page->prosses > bitmap->size)
    {
        log_crit(MODULE, "page id %u is over %u", page->prosses, num_pages);
    }

    // Create a new heap block
    HeapBlock *new_block = NULL;
    if (page->heapBlock == NULL)
    {
        // FirstBlock
        size_t pageIndex = (page - pages) + 1;
        new_block = FirstBlock + (pageIndex * PAGE_BLOCKSPACE);
    }
    else
    {
        size_t pageIndex = (page - pages) + 1;
        HeapBlock *start = FirstBlock + (pageIndex * PAGE_BLOCKSPACE);
        int blockIndex = (page->heapBlock - start) / sizeof(HeapBlock) + 1;
        new_block = (HeapBlock *)((uint8_t *)(page->heapBlock) + sizeof(HeapBlock));
    }

    new_block->size = size;
    new_block->data = (uint8_t *)heap_start; // Assign memory for the block's data
    heap_start += size;
    // log_debug(MODULE, "newBlock: 0x%p &data 0x%p, size %u", new_block, new_block->data, new_block->size);

    // Link the new block to the page's heapBlock linked list
    new_block->nextBlock = page->heapBlock;
    page->heapBlock = new_block;

    if (page->heapBlock->nextBlock != NULL)
    {
        log_info(MODULE, "In while: current=%p, nextBlock=%p", page->heapBlock, page->heapBlock->nextBlock);
    }

    // log_info(MODULE, "page id %u, allocBlocks: %u", page->prosses, page->allocatedBlocks);
    uint32_t allocBlock = page->allocatedBlocks + 1;
    page->allocatedBlocks = allocBlock;
    memoryAllocated += size;
    // log_info(MODULE, "page id %u, allocBlocks: %u", page->prosses, page->allocatedBlocks);

    // log_debug(MODULE, "heap_start = 0x%p", heap_start);
    // log_debug(MODULE, "newBlock: 0x%p &data 0x%p, size %u", new_block, new_block->data, new_block->size);
    // log_debug(MODULE, "return pointer address 0x%p", new_block->data);

    return (void *)new_block->data; // Return the pointer to the usable memory
}

// Function to free a block of memory from a given page
void heap_free(Page *page, void *ptr)
{
    if (!page || !ptr)
    {
        return; // Nothing to free if page or pointer is NULL
    }

    HeapBlock *current = page->heapBlock;
    HeapBlock *prev = NULL;

    while (current)
    {
        if ((void *)current->data == ptr)
        {
            // Remove the block from the linked list
            if (prev)
            {
                prev->nextBlock = current->nextBlock;
            }
            else
            {
                page->heapBlock = current->nextBlock;
            }

            memoryAllocated -= current->size;
            page->allocatedBlocks--;

            // If the page has no more allocated blocks, free the page
            if (page->allocatedBlocks == 0)
            {
                free_page(page);
            }

            return; // Successfully freed
        }

        prev = current;
        current = current->nextBlock;
    }
}
