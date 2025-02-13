#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint16_t general_config; // Word 0: General configuration (bit flags)
    uint16_t cylinders;      // Word 1: Number of logical cylinders (CHS)
    uint16_t reserved_2;
    uint16_t heads; // Word 3: Number of logical heads (CHS)
    uint16_t reserved_4;
    uint16_t sectors_per_track; // Word 6: Number of logical sectors per track (CHS)
    uint16_t reserved_7_9[3];
    char serial_number[20]; // Words 10-19: Serial number (ASCII)
    uint16_t reserved_20_22[3];
    char firmware_revision[8];    // Words 23-26: Firmware revision (ASCII)
    char model_number[40];        // Words 27-46: Model number (ASCII)
    uint16_t maximum_rw_multiple; // Word 47: Maximum sectors per R/W multiple
    uint16_t reserved_48;
    uint32_t capabilities; // Words 49-50: Capabilities flags
    uint32_t reserved_51_52;
    uint16_t field_validity;           // Word 53: Field validity flags
    uint16_t current_cylinders;        // Word 54: Current CHS cylinders
    uint16_t current_heads;            // Word 55: Current CHS heads
    uint16_t current_sectors;          // Word 56: Current CHS sectors
    uint32_t current_capacity;         // Words 57-58: Current CHS capacity
    uint32_t lba_sector_count;         // Words 60-61: Total LBA sectors (28-bit)
    uint16_t multiword_dma;            // Word 63: Multiword DMA modes supported
    uint16_t pio_modes;                // Word 64: PIO modes supported
    uint16_t min_dma_cycle_time;       // Word 65: Minimum DMA cycle time
    uint16_t recommended_dma_cycle;    // Word 66: Recommended DMA cycle time
    uint16_t min_pio_cycle_time;       // Word 67: Minimum PIO cycle time (without flow control)
    uint16_t min_pio_cycle_time_iordy; // Word 68: Minimum PIO cycle time (with IORDY flow control)
    uint16_t reserved_69_74[6];
    uint16_t queue_depth;             // Word 75: Queue depth (NCQ support)
    uint16_t sata_capabilities;       // Word 76: SATA capabilities
    uint16_t sata_features_supported; // Word 77: SATA features supported
    uint32_t reserved_78_79;
    uint16_t major_version;             // Word 80: ATA Major version number
    uint16_t minor_version;             // Word 81: ATA Minor version number
    uint16_t command_sets_supported[3]; // Words 82-84: Supported command sets (bitmask)
    uint16_t command_sets_enabled[3];   // Words 85-87: Enabled command sets (bitmask)
    uint16_t ultra_dma_modes;           // Word 88: Ultra DMA mode support
    uint16_t reserved_89_99[11];
    uint32_t lba48_sector_count; // Words 100-103: 48-bit LBA sector count
    uint16_t reserved_104_126[23];
    uint16_t security_status; // Word 128: Security status (drive locked, frozen, etc.)
    uint16_t reserved_129_159[31];
    uint16_t apm_value; // Word 160: Advanced Power Management (APM)
    uint16_t reserved_161_191[31];
    uint16_t aam_value; // Word 192: Acoustic Management (AAM)
    uint16_t reserved_193_254[62];
    uint16_t integrity_word; // Word 255: Integrity checksum
} __attribute__((packed)) ATA_Identify_t;

typedef struct ahci_command_list_entry_t
{
    uint8_t command_fis_length_in_dwords : 5;
    uint8_t atapi : 1;
    uint8_t write : 1;
    uint8_t prefetchable : 1;
    uint8_t reset : 1;
    uint8_t bist : 1;
    uint8_t clear : 1;
    uint8_t reserved1 : 1;
    uint8_t port_multiplier : 4;
    uint16_t number_of_command_table_entries;
    uint32_t byte_count;
    uint32_t command_table_low_memory;
    uint32_t command_table_high_memory;
    uint8_t reserved2[16];
} __attribute__((packed)) ahci_command_list_entry_t;

typedef struct ahci_command_and_prd_table_t
{
    uint8_t fis_type;
    uint8_t flags;
    uint8_t command;
    uint8_t features;
    uint8_t lba_0;
    uint8_t lba_1;
    uint8_t lba_2;
    uint8_t device_head;
    uint8_t lba_3;
    uint8_t lba_4;
    uint8_t lba_5;
    uint8_t features_expanded;
    uint8_t sector_count_low;
    uint8_t sector_count_high;
    uint8_t reserved1;
    uint8_t control;
    uint8_t reserved2[0x30];

    uint8_t atapi_command[0x10];
    uint8_t reserved3[0x30];

    uint32_t data_base_low_memory;
    uint32_t data_base_high_memory;
    uint32_t reserved4;
    uint32_t data_byte_count;
} __attribute__((packed)) ahci_command_and_prd_table_t;

void ATA_init();
bool ATA_identify(ATA_Identify_t *buffer);
uint32_t ATA_read(uint8_t *buf, uint32_t lba, uint32_t numsects, uint16_t drive);