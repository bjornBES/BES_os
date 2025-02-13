
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
void *malloc(size_t size) {
    if (size == 0) {
        return NULL;  // Return NULL for zero-size allocation
    }

    // Find a free page for allocation
    void *page = allocate_page();
    if (page != NULL) {
        // Initialize the page's linked list
        init_page((Page *)page);

        // Allocate memory inside the page
        return page_alloc((Page *)page, size);
    }

    log_crit(MODULE, "OUT OF MEMORY");
    return NULL;  // No pages left
}

// Function to free memory (assumes the block belongs to a page)
void free(void *ptr) {
    if (!ptr) return;

    // Find the page the block belongs to (aligned to page boundary)
    uint32_t u8ptr = (uint32_t)ptr & (~(PAGE_SIZE - 1));
    uint8_t* pu8ptr = (uint8_t*)pu8ptr;
    Page *page = (Page *)(pu8ptr);

    // Free the block inside the page
    page_free(page, ptr);
}

// Function to allocate memory and zero it (calloc)
void *calloc(size_t num, size_t size) {
    size_t total_size = num * size;
    void *ptr = malloc(total_size);
    if (ptr) {
        // Zero the allocated memory
        for (size_t i = 0; i < total_size; i++) {
            ((uint8_t *)ptr)[i] = 0;
        }
    }
    return ptr;
}

// Function to resize allocated memory (realloc)
void *realloc(void *ptr, size_t size) {
    if (!ptr) {
        return malloc(size);  // If ptr is NULL, realloc behaves like malloc
    }

    if (size == 0) {
        free(ptr);  // If size is 0, realloc behaves like free
        return NULL;
    }

    // Since we are not handling memory copying in this simple version,
    // this part would require more advanced management for full support.
    return malloc(size);  // For now, just treat realloc as malloc.
}
