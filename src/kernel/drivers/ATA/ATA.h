#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "drivers/device.h"

typedef struct {
    uint16_t general_config;                    // word: 0
    uint16_t cylinders;                         // word: 1
    uint16_t specific_config;                   // word: 2
    uint16_t heads;                             // word: 3
    uint16_t reserved4;                         // word: 4
    uint16_t sectors_per_track;                 // word: 5
    uint16_t reserved6_9[3];                    // words: 6-9
    char serial_number[20];                     // 10–19 (ASCII)
    uint16_t reserved_20_22[3];                 // 20–22
    char firmware_revision[8];                  // 23–26 (ASCII)
    char model_number[40];                      // 27–46 (ASCII)
    uint16_t max_sectors_per_interrupt;         // 47
    uint16_t reserved_48;
    uint32_t capabilities;                      // Words 49-50: Capabilities flags
    uint16_t pio_timing;                        // 51
    uint16_t dma_timing;                        // 52
    uint16_t field_validity;                    // 53
    uint16_t current_cylinders;                 // 54
    uint16_t current_heads;                     // 55
    uint16_t current_sectors_per_track;         // 56
    uint32_t current_capacity;                  // 57–58 (not reliable)
    uint16_t multi_sector_settings;             // 59
    uint32_t lba28_total_sectors;               // 60–61
    uint16_t single_word_dma;                   // 62
    uint16_t multi_word_dma;                    // 63
    uint16_t advanced_pio_modes;                // 64
    uint16_t min_dma_cycle_time;                // 65
    uint16_t recommended_dma_cycle_time;        // 66
    uint16_t min_pio_cycle_time_no_flow;        // 67
    uint16_t min_pio_cycle_time_with_flow;      // 68
    uint16_t reserved1[6];                      // 69–74
    uint16_t queue_depth;                       // 75
    uint16_t sata_capabilities;                 // 76 ← [AHCI relevant]
    uint32_t sata_reserved;                     // 77–78
    uint16_t sata_features_supported;           // 79
    uint16_t ata_major_version;                 // 80
    uint16_t ata_minor_version;                 // 81
    uint16_t command_sets_supported[3];         // 82–84
    uint16_t command_sets_enabled[3];           // 85–87
    uint16_t ultra_dma_modes;                   // 88
    uint16_t time_required_erase;               // 89
    uint16_t time_required_enhanced_erase;      // 90
    uint16_t current_apm_value;                 // 91
    uint16_t master_password_revision;          // 92
    uint16_t hw_reset_result;                   // 93
    uint16_t acoustic_management;               // 94
    uint16_t stream_minimum_req_size;           // 95
    uint16_t stream_transfer_time_dma;          // 96
    uint16_t stream_access_latency_dma;         // 97
    uint32_t stream_perf_granularity;           // 98–99
    uint64_t lba48_total_sectors;               // 100–103 ← [Use this for LBA48 drives]
    uint16_t streaming_transfer_time;           // 104
    uint16_t dsm_features;                      // 105
    uint16_t phys_log_sector_info;              // 106
    uint16_t inter_seek_delay;                  // 107
    uint64_t world_wide_name;                   // 108–111
    uint64_t reserved2;                         // 112–115
    uint16_t logical_sector_size_info;          // 117
    uint32_t commands_supported_2;              // 118–119
    uint32_t commands_enabled_2;                // 120–121
    uint16_t reserved3[6];                      // 122–127
    uint16_t security_status;                   // 128
    uint16_t vendor_specific[31];               // 129–159
    uint16_t reserved5[96];                     // 160–255
} __attribute__((packed)) IdentifyDeviceData;

extern uint16_t ata_Device;

void ATA_init();
bool ATA_identify(IdentifyDeviceData *buffer);
bool ATA_read(uint8_t *buf, uint32_t lba, uint32_t numsects, struct __device_t* drive);