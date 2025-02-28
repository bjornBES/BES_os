#pragma once

#include <stdint.h>
#include <stdbool.h>

#define MEMORY_AVAILABLE              1
#define MEMORY_RESERVED               2
#define MEMORY_ACPI_RECLAIMABLE       3
#define MEMORY_NVS                    4
#define MEMORY_BADRAM                 5

typedef struct {
    uint16_t attributes;
    uint8_t winA, winB;
    uint16_t granularity;
    uint16_t winsize;
    uint16_t segmentA, segmentB;
    uint32_t real_fptr;
    uint16_t pitch;
    uint16_t x_res, y_res;
    uint8_t w_char, y_char, planes, bpp, banks, memory_model, bank_size, image_pages;
    uint8_t reserved0;
    uint8_t red_mask, red_position;
    uint8_t green_mask, green_position;
    uint8_t blue_mask, blue_position;
    uint8_t rsv_mask, rsv_position;
    uint8_t directcolor_attributes;
    uint32_t framebuffer;  /* Linear framebuffer address */
    uint32_t offscreen_mem;
    uint16_t offscreen_size;
    uint8_t reserved1[206];
} __attribute__((packed)) vesa_mode_info_t;

typedef struct {
    uint64_t Begin, Length;
    uint32_t Type;
    uint32_t ACPI;
} MemoryRegion;

typedef struct  {
    int RegionCount;
    MemoryRegion* Regions;
} MemoryInfo;

typedef struct {
    bool SupportVESA;
    int VESACount;
    vesa_mode_info_t* entries;
} VESAInfo;

typedef struct {
    MemoryInfo Memory;
    uint8_t BootDevice;
    VESAInfo VESA;
    
} BootParams;