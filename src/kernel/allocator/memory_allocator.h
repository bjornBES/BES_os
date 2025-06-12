#pragma once

#include "defaultInclude.h"

// Define the size of a single page (typically 4KB)
#define PAGE_SIZE 4096

#define PGROUNDUP(sz)  (((sz)+PAGE_SIZE-1) & ~(PAGE_SIZE-1))
#define PGROUNDDOWN(a) (((a)) & ~(PAGE_SIZE-1))

typedef struct HeapBlock
{
    size_t size;
    struct HeapBlock* nextBlock;
    uint8_t* data;
} HeapBlock;
typedef struct Page
{
    HeapBlock* heapBlock;
    uint32_t allocatedBlocks;
    int32_t prosses;
} Page;

// Bitmap structure to manage page allocation
typedef struct PageBitmap {
    uint32_t *bitmap;  // The actual bitmap array
    size_t size;       // Number of bits (pages) we can track
} PageBitmap;

extern PageBitmap *bitmap;
extern Page *pages;
extern size_t num_pages;

// Function declarations
void mm_init();
Page* allocate_page();
void free_page(Page*page);
void dump_memory_status();
void *heap_alloc(Page *page, size_t size);
void heap_free(Page *page, void *ptr);
Page *GetPage(int pageIndex);

// Bitmap management functions
void set_bit(size_t index);
void clear_bit(size_t index);
bool is_bit_set(size_t index);

void mm_test();