#include "memory_allocator.h"
#include "debug.h"
#include "memory.h"

#include <string.h>

#define MODULE "ALLOC"

extern char __end;
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
int32_t pageProsses;

#define MAX_VALUE (256 * 4096)*4  // 4MB

// Function to initialize the bitmap and linked list system
void mm_init()
{
    pageProsses = 0;
    pagesAllocated = 0;
    memoryAllocated = 0;
    
    heap_start = (uint8_t*)&__end + MAX_VALUE;
    memory_Start = (uint8_t*)&__end;
    memory_End = memory_Start + MAX_VALUE - 1;
    
    total_size = (size_t)((memory_End + 1) - memory_Start);
    num_pages = total_size / PAGE_SIZE; // Calculate number of pages
    // Allocate memory for the bitmap struct
    bitmap = (PageBitmap *)memory_Start; // Point bitmap to the start of the memory region
    
    // Initialize the bitmap struct
    bitmap->size = num_pages;
    bitmap->bitmap = (uint32_t *)(memory_Start + sizeof(PageBitmap)); // The bitmap starts after the struct
    
    // Initialize all bits to 0 (free)
    memset(bitmap->bitmap, 0, (num_pages / 32) + 1);
    
    // Initialize the pages
    pages = (Page *)((uint8_t *)memory_Start + sizeof(PageBitmap) + (num_pages / 32) + 1);
    for (size_t i = 0; i < num_pages; i++)
    {
        pages[i].heapBlock = NULL;
        pages[i].allocatedBlocks = 0;
        pages[i].prosses = -1;
    }

    log_debug(MODULE, "__end = 0x%p", &__end);
    log_debug(MODULE, "heap start 0x%p", heap_start);
    log_debug(MODULE, "memory start: 0x%p, memory_End: 0x%p, memory Size: 0x%p", memory_Start, memory_End, memory_End - memory_Start);
    log_debug(MODULE, "total size 0x%X", total_size);
    log_debug(MODULE, "num page %u", num_pages);
    log_debug(MODULE, "bitmap: 0x%p, &size: 0x%p, &bitmap addr: 0x%p", bitmap, &bitmap->size, bitmap->bitmap);
    log_debug(MODULE, "pages: 0x%p", pages);

    log_info(MODULE, "sizeof HeapBlock = %u", sizeof(HeapBlock));
    log_info(MODULE, "sizeof Page = %u", sizeof(Page));
    log_info(MODULE, "sizeof PageBitmap = %u", sizeof(PageBitmap));

    // allocating page 0 for general stuff
    allocate_page();
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
    // log_warn(MODULE, "New page pageProsses: %u, pagesAllocated: %u", pageProsses, pagesAllocated);
    // log_debug(MODULE, "In allocate_page function");
    size_t num_pages = bitmap->size;
    
    for (size_t i = 0; i < num_pages; i++)
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
            // log_debug(MODULE, "page: 0x%p", page);
            pageProsses++;
            return page;
        }
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
            log_info("MEMORY DUMP", "- pages[%u]: 0x%p heapBlock(0x%p), allocatedBlocks: %x pageProsses:%u", i, &pages[i], &pages[i].heapBlock, pages[i].allocatedBlocks, pages[i].prosses);
            HeapBlock *current_block = page->heapBlock;
            int count = 0;
            while (current_block != NULL)
            {
                used_bytes += current_block->size;
                log_info("MEMORY DUMP", "- current_block[%u]: 0x%p size(0x%p):%u, Next(0x%p), data(0x%p)", count, current_block, &(current_block->size), current_block->size, current_block->nextBlock, current_block->data);
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
void *heap_alloc(Page *page, size_t size) {
    // Check if the requested size is valid
    // log_debug(MODULE, "page addr: 0x%p, size = %u heap_start = 0x%p", page, size, heap_start);
    if (size == 0 || size > PAGE_SIZE) 
    {
        log_err(MODULE, "return NULL in heap_alloc");
        return NULL; // Cannot allocate if the size is invalid or exceeds a page size
    }
    
    // Create a new heap block
    HeapBlock *new_block = (HeapBlock *)heap_start;
    heap_start += sizeof(HeapBlock); // Move the heap_start pointer to make space for the new block
    
    new_block->size = size;
    new_block->data = (uint8_t *)heap_start; // Assign memory for the block's data
    heap_start += size;
    // log_debug(MODULE, "newBlock: 0x%p &data 0x%p, size %u", new_block, new_block->data, new_block->size);
    
    // Link the new block to the page's heapBlock linked list
    new_block->nextBlock = page->heapBlock;
    page->heapBlock = new_block;
    
    log_warn(MODULE, "page id %u, allocBlocks: %u", page->prosses, page->allocatedBlocks);
    uint32_t allocBlock = page->allocatedBlocks + 1; 
    page->allocatedBlocks = allocBlock;
    memoryAllocated += size;
    log_warn(MODULE, "page id %u, allocBlocks: %u", page->prosses, page->allocatedBlocks);
    
    // log_debug(MODULE, "heap_start = 0x%p", heap_start);
    // log_debug(MODULE, "newBlock: 0x%p &data 0x%p, size %u", new_block, new_block->data, new_block->size);
    // log_debug(MODULE, "return pointer address 0x%p", new_block->data);

    return (void *)new_block->data; // Return the pointer to the usable memory
}

// Function to free a block of memory from a given page
void heap_free(Page *page, void *ptr) {
    if (!page || !ptr) {
        return; // Nothing to free if page or pointer is NULL
    }

    HeapBlock *current = page->heapBlock;
    HeapBlock *prev = NULL;

    while (current) {
        if ((void *)current->data == ptr) {
            // Remove the block from the linked list
            if (prev) {
                prev->nextBlock = current->nextBlock;
            } else {
                page->heapBlock = current->nextBlock;
            }

            memoryAllocated -= current->size;
            page->allocatedBlocks--;

            // If the page has no more allocated blocks, free the page
            if (page->allocatedBlocks == 0) {
                free_page(page);
            }

            return; // Successfully freed
        }

        prev = current;
        current = current->nextBlock;
    }
}

