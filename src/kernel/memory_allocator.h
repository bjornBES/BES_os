#pragma once

#include <stddef.h>  // For size_t
#include <stdint.h>
#include <boot/bootparams.h>

// Define the size of a single page (typically 4KB)
#define PAGE_SIZE 4096

// Bitmap structure to manage page allocation
typedef struct PageBitmap {
    uint32_t *bitmap;  // The actual bitmap array
    size_t size;       // Number of bits (pages) we can track
} PageBitmap;

// Block structure for the linked list inside a page
typedef struct Block {
    size_t size;          // Size of the block in bytes
    struct Block *next;   // Pointer to the next free block
} Block;

// Page structure to hold the free list and the page data
typedef struct Page {
    Block *free_list;     // The head of the free list
    uint8_t data[PAGE_SIZE];  // The actual data of the page (size is PAGE_SIZE)
} Page;

// Function declarations
void mm_init(MemoryRegion memRegion);
void *allocate_page(void);
void free_page(void *page_address);
void *page_alloc(Page *page, size_t size);
void page_free(Page *page, void *ptr);
void init_page(Page *page);
void dump_memory_status();