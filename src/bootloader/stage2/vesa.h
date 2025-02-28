#pragma once

#include <stdint.h>
#include <boot/bootparams.h>

typedef struct {
    char signature[4];
    uint16_t version;
    char* OEM;
    uint32_t flags;
    uint16_t XGAAdapters;
    uint8_t reserved[240];
} __attribute__((packed)) XGAInfo;

void Detect_VESA(VESAInfo* vesaInfo);
