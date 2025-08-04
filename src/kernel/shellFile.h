#pragma once

#include "defaultInclude.h"

typedef union CexeHeader
{
    struct{
        char magic[4];
        uint8_t version;
        uint32_t programLength;
        uint32_t physOffset;
        uint8_t reserved[19];
    } __attribute__((packed)) Header;
    uint8_t bytes[32];
} __attribute__((packed)) CexeHeader_t;
typedef union CexeSectionEntry
{
    struct{
        char ident[2];
        uint8_t flags;
        union
        {
            uint64_t offset;
            uint32_t u32Offset[2];
        } offset;
        uint32_t size;
        uint8_t reserved;
    } __attribute__((packed)) section;
    uint8_t bytes[16];
} __attribute__((packed)) CexeSectionEntry_t;
typedef struct CexeProgramHeader
{
    CexeHeader_t header;
    CexeSectionEntry_t entries[4];
} __attribute__((packed)) CexeProgramHeader_t;

void EnterShell();