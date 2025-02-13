#include "memory_allocator.h"
#include <stdint.h>
#include <stddef.h> // For size_t

#include "debug.h"

#define MODULE "ALLOC"

// Bitmap structure to manage page allocation
static PageBitmap *bitmap = NULL;
uint8_t *memory_Start = NULL; // Start of memory
uint8_t *memory_End = NULL;   // End of memory
uint32_t memoryAllocated;
uint32_t pagesAllocated;

// Function to initialize the bitmap and linked list system
void mm_init(MemoryRegion memRegion)
{
    pagesAllocated = 0;
    memoryAllocated = 0;

    memory_Start = (uint8_t *)((uint32_t)memRegion.Begin);
    if (memory_Start == 0)
    {
        memory_Start = (uint8_t *)PAGE_SIZE;
    }
    memory_End = memory_Start + memRegion.Length;

    size_t total_size = (size_t)(memory_End - memory_Start);
    size_t num_pages = total_size / PAGE_SIZE; // Calculate number of pages

    // Allocate memory for the bitmap struct
    bitmap = (PageBitmap *)memory_Start; // Point bitmap to the start of the memory region

    // Initialize the bitmap struct
    bitmap->size = num_pages;
    bitmap->bitmap = (uint32_t *)(memory_Start + sizeof(PageBitmap)); // The bitmap starts after the struct

    // Initialize all bits to 0 (indicating all pages are free)
    for (size_t i = 0; i < num_pages / 32; i++)
    {
        bitmap->bitmap[i] = 0;
    }

    // If the number of pages isn't a multiple of 32, clear the remaining bits
    if (num_pages % 32)
    {
        bitmap->bitmap[num_pages / 32] = 0;
    }
}

void dump_memory_status()
{
	size_t num_pages = bitmap->size;
    size_t allocated_pages = 0;
    size_t used_bytes = 0;

    // Count allocated pages
    for (size_t i = 0; i < num_pages; i++) {
        size_t bit_index = i % 32;
        size_t word_index = i / 32;

        if (bitmap->bitmap[word_index] & (1 << bit_index)) {
            allocated_pages++;

            // Check the page's internal allocation
            Page *page = (Page *)(memory_Start + (i * PAGE_SIZE));
            Block *block = page->free_list;

            // Calculate used memory inside this allocated page
            size_t free_memory = 0;
            while (block) {
                free_memory += block->size + sizeof(Block);
                block = block->next;
            }
            used_bytes += (PAGE_SIZE - free_memory);
        }
    }

    size_t free_pages = num_pages - allocated_pages;

    printf("[MEMORY DUMP] Total pages: %u\n", num_pages);
    printf("[MEMORY DUMP] Allocated pages: %u\n", allocated_pages);
    printf("[MEMORY DUMP] Free pages: %u\n", free_pages);
    printf("[MEMORY DUMP] Used bytes in allocated pages: %u\n", used_bytes);
}

// Allocate a page using the bitmap
void *allocate_page(void)
{
    size_t num_pages = bitmap->size;

    // Loop over each bit in the bitmap to find the first free page
    for (size_t i = 0; i < num_pages; i++)
    {
        size_t bit_index = i % 32;  // Find the bit position within the 32-bit word
        size_t word_index = i / 32; // Which 32-bit word to look in

        if (!(bitmap->bitmap[word_index] & (1 << bit_index)))
        {
            memoryAllocated += PAGE_SIZE;
            // log_debug(MODULE, "got bit nr %u", bit_index);
            // Mark the page as allocated
            bitmap->bitmap[word_index] |= (1 << bit_index);

            // Return the address of the allocated page
            return memory_Start + (i * PAGE_SIZE);
        }
    }
    log_crit(MODULE, "Didnt find page");
    return NULL; // No free pages left
}

// Free a page by marking the bitmap entry as free
void free_page(void *page_address)
{
    size_t page_index = ((uint8_t *)page_address - memory_Start) / PAGE_SIZE;

    size_t bit_index = page_index % 32;
    size_t word_index = page_index / 32;

    // log_debug(MODULE, "got bit nr %u", bit_index);
    memoryAllocated -= PAGE_SIZE;
    // Mark the page as free
    bitmap->bitmap[word_index] &= ~(1 << bit_index);
}

// Initialize the linked list inside a page
void init_page(Page *page)
{
    // log_debug(MODULE, "init page %p", page);
    // Create a single large free block that covers the entire page
    page->free_list = (Block *)page->data;
    page->free_list->size = PAGE_SIZE - sizeof(Block); // Leave space for the Block structure itself
    page->free_list->next = NULL;                      // No other free blocks yet
}

// Allocate a block inside a page
void *page_alloc(Page *page, size_t size)
{
    Block *prev = NULL;
    Block *curr = page->free_list;

    // Traverse the free list to find a large enough block
    while (curr)
    {
        if (curr->size >= size)
        {
            // If the block is larger than needed, split it
            if (curr->size > size + sizeof(Block))
            {
                // Split the block
                Block *new_block = (Block *)((uint8_t *)curr + size + sizeof(Block));
                new_block->size = curr->size - size - sizeof(Block);
                new_block->next = curr->next;

                curr->size = size;
                curr->next = new_block;
            }

            // Remove the block from the free list
            if (prev)
            {
                prev->next = curr->next;
            }
            else
            {
                page->free_list = curr->next;
            }

            // log_debug(MODULE, "got addr page %p", (uint8_t *)curr + sizeof(Block));
            return (void *)((uint8_t *)curr + sizeof(Block)); // Return memory after the Block header
        }

        prev = curr;
        curr = curr->next;
    }

    return NULL; // No free block large enough
}

// Free a block inside a page
void page_free(Page *page, void *ptr)
{
    // Get the Block structure that corresponds to the given pointer
    Block *block = (Block *)((uint8_t *)ptr - sizeof(Block));

    // log_debug(MODULE, "freeing block %p", block);

    // Add the block back to the free list
    block->next = page->free_list;
    page->free_list = block;
}
