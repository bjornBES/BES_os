
#include "memory.h"
#include "stdio.h"
#include "debug.h"
#include "malloc.h"

#define MODULE "MALLOC"

void printStatus()
{
    log_debug(MODULE, "file %s:%u", __FILE__, __LINE__);
    dump_memory_status();
}

// Function to allocate memory (with basic error checking)
void *malloc(size_t size, Page *page)
{
    if (size == 0)
    {
        log_err(MODULE, "return NULL");
        return NULL; // Return NULL for zero-size allocation
    }
    // log_info(MODULE, "is page null? %s", page == NULL ? "TRUE" : "FALSE");
    if (page == NULL)
    {
        page = allocate_page(); // Allocate the page
        if (page == NULL)
        {
            log_crit(MODULE, "OUT OF MEMORY");
            return NULL; // No pages left
        }
    }
    // log_info(MODULE, "got page page: %p null? %s", page, page == 0 ? "TRUE" : "FALSE");
    // Find the first available page
    if (page != NULL)
    {
        void *data = heap_alloc(page, size);
        // log_info(MODULE, "got page page: %p null? %s", page, page == 0 ? "TRUE" : "FALSE");
        return data;
    }
    log_crit(MODULE, "OUT OF MEMORY");
    return NULL; // No pages left
}

// Function to free memory (assumes the block belongs to a page)
void free(void *ptr, Page *page)
{
    if (!ptr)
    {
        return;
    }
    // log_debug(MODULE, "page addr: %p, page.prosses: %u, page.allocatedBlocks: %u, page.heapBlock: 0x%p", page, page->prosses, page->allocatedBlocks, page->heapBlock);
    int i = 0;
    // Find the page this pointer belongs to
    HeapBlock *current_block = page->heapBlock;
    // log_debug(MODULE, "%u: current_block: & %p, data %p, next %p, size %u", i, current_block, current_block->data, current_block->nextBlock, current_block->size);
    // Find the block with the given pointer and free it
    while (current_block != NULL)
    {
        // log_debug(MODULE, "%u: current_block: & %p, data %p, next %p, size %u", i, current_block, current_block->data, current_block->nextBlock, current_block->size);
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
        i++;
        continue;
    }
}

// Function to allocate memory and zero it (calloc)
void *calloc(size_t num, size_t size, Page *page)
{
    size_t total_size = num * size;
    void *ptr = malloc(total_size, page);
    if (ptr)
    {
        memset(ptr, 0, size);
        return ptr;
    }
    return NULL;
}

// Function to resize allocated memory (realloc)
void *realloc(void *ptr, size_t size, Page *page)
{
    if (!ptr)
    {
        return malloc(size, page); // If ptr is NULL, realloc behaves like malloc
    }

    if (size == 0)
    {
        free(ptr, page); // If size is 0, realloc behaves like free
        return NULL;
    }

    // Since we are not handling memory copying in this simple version,
    // this part would require more advanced management for full support.
    return malloc(size, page); // For now, just treat realloc as malloc.
}

void *mallocToPage0(size_t size)
{
    Page* page = &pages[0];
    if (size == 0)
    {
        log_err(MODULE, "return NULL");
        return NULL; // Return NULL for zero-size allocation
    }
    // log_info(MODULE, "is page null? %s", page == NULL ? "TRUE" : "FALSE");
    if (page == NULL)
    {
        page = allocate_page(); // Allocate the page
        if (page == NULL)
        {
            log_crit(MODULE, "OUT OF MEMORY");
            return NULL; // No pages left
        }
    }
    // log_info(MODULE, "got page page: %p null? %s", page, page == 0 ? "TRUE" : "FALSE");
    // Find the first available page
    if (page != NULL)
    {
        void *data = heap_alloc(page, size);
        // log_info(MODULE, "got page page: %p null? %s", page, page == 0 ? "TRUE" : "FALSE");
        return data;
    }
    log_crit(MODULE, "OUT OF MEMORY");
    return NULL; // No pages left
}

void *mallocToPage(size_t size, int pageIndex)
{
    Page* page = &pages[pageIndex];
    if (size == 0)
    {
        log_err(MODULE, "return NULL");
        return NULL; // Return NULL for zero-size allocation
    }
    // log_info(MODULE, "is page null? %s", page == NULL ? "TRUE" : "FALSE");
    if (page == NULL)
    {
        page = allocate_page(); // Allocate the page
        if (page == NULL)
        {
            log_crit(MODULE, "OUT OF MEMORY");
            return NULL; // No pages left
        }
    }
    // log_info(MODULE, "got page page: %p null? %s", page, page == 0 ? "TRUE" : "FALSE");
    // Find the first available page
    if (page != NULL)
    {
        void *data = heap_alloc(page, size);
        // log_info(MODULE, "got page page: %p null? %s", page, page == 0 ? "TRUE" : "FALSE");
        return data;
    }
    log_crit(MODULE, "OUT OF MEMORY");
    return NULL; // No pages left
}

void freeToPage(void* ptr, int pageIndex)
{
    Page* page = &pages[pageIndex];
    if (!ptr)
    {
        return;
    }
    // log_debug(MODULE, "page addr: %p, page.prosses: %u, page.allocatedBlocks: %u, page.heapBlock: 0x%p", page, page->prosses, page->allocatedBlocks, page->heapBlock);
    int i = 0;
    // Find the page this pointer belongs to
    HeapBlock *current_block = page->heapBlock;
    // log_debug(MODULE, "%u: current_block: & %p, data %p, next %p, size %u", i, current_block, current_block->data, current_block->nextBlock, current_block->size);
    // Find the block with the given pointer and free it
    while (current_block != NULL)
    {
        // log_debug(MODULE, "%u: current_block: & %p, data %p, next %p, size %u", i, current_block, current_block->data, current_block->nextBlock, current_block->size);
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
        i++;
        continue;
    }
}
