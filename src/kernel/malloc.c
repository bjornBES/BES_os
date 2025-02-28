
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "memory.h"
#include "stdio.h"
#include "debug.h"
#include "malloc.h"
#include "memory_allocator.h"

#define MODULE "MALLOC"

void printStatus()
{
    dump_memory_status();
}

// Function to allocate memory (with basic error checking)
void *malloc(size_t size)
{
    if (size == 0)
    {
        return NULL; // Return NULL for zero-size allocation
    }

    // Find the first available page
    for (size_t i = 0; i < num_pages; i++)
    {
        if (!is_bit_set(i)) // If the page is not allocated
        {
            return heap_alloc(&pages[i], size);
        }
    }
    Page *page = allocate_page(); // Allocate the page
    if (page == NULL)
    {
        return heap_alloc(page, size);
    }
    log_crit(MODULE, "OUT OF MEMORY");
    return NULL; // No pages left
}

// Function to free memory (assumes the block belongs to a page)
void free(void *ptr)
{
    if (!ptr)
        return;

    // Find the page this pointer belongs to
    for (size_t i = 0; i < num_pages; i++)
    {
        Page *page = &pages[i];
        HeapBlock *current_block = page->heapBlock;
        // Find the block with the given pointer and free it
        while (current_block != NULL)
        {
            if (current_block->data != 0)
            {
                if (current_block->data == ptr)
                {
                    heap_free(page, ptr);
                    ptr = NULL;
                    return;
                }
            }
            current_block = current_block->nextBlock;
            continue;
        }
    }
}

// Function to allocate memory and zero it (calloc)
void *calloc(size_t num, size_t size)
{
    size_t total_size = num * size;
    void *ptr = malloc(total_size);
    if (ptr)
    {
        // Zero the allocated memory
        for (size_t i = 0; i < total_size; i++)
        {
            ((uint8_t *)ptr)[i] = 0;
        }
        return ptr;
    }
    return NULL;
}

// Function to resize allocated memory (realloc)
void *realloc(void *ptr, size_t size)
{
    if (!ptr)
    {
        return malloc(size); // If ptr is NULL, realloc behaves like malloc
    }

    if (size == 0)
    {
        free(ptr); // If size is 0, realloc behaves like free
        return NULL;
    }

    // Since we are not handling memory copying in this simple version,
    // this part would require more advanced management for full support.
    return malloc(size); // For now, just treat realloc as malloc.
}
