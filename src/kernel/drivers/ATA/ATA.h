#pragma once

#include <stdint.h>

#define ATA_SR_BSY     0x80
#define ATA_SR_DRDY    0x40
#define ATA_SR_DF      0x20
#define ATA_SR_DSC     0x10
#define ATA_SR_DRQ     0x08
#define ATA_SR_CORR    0x04
#define ATA_SR_IDX     0x02
#define ATA_SR_ERR     0x01

#define ATA_ER_BBK      0x80
#define ATA_ER_UNC      0x40
#define ATA_ER_MC       0x20
#define ATA_ER_IDNF     0x10
#define ATA_ER_MCR      0x08
#define ATA_ER_ABRT     0x04
#define ATA_ER_TK0NF    0x02
#define ATA_ER_AMNF     0x01

#define ATA_CMD_READ_PIO          0x20
#define ATA_CMD_READ_PIO_EXT      0x24
#define ATA_CMD_READ_DMA          0xC8
#define ATA_CMD_READ_DMA_EXT      0x25
#define ATA_CMD_WRITE_PIO         0x30
#define ATA_CMD_WRITE_PIO_EXT     0x34
#define ATA_CMD_WRITE_DMA         0xCA
#define ATA_CMD_WRITE_DMA_EXT     0x35
#define ATA_CMD_CACHE_FLUSH       0xE7
#define ATA_CMD_CACHE_FLUSH_EXT   0xEA
#define ATA_CMD_PACKET            0xA0
#define ATA_CMD_IDENTIFY_PACKET   0xA1
#define ATA_CMD_IDENTIFY          0xEC

#define      ATAPI_CMD_READ       0xA8
#define      ATAPI_CMD_EJECT      0x1B

#define ATA_IDENT_DEVICETYPE   0x00
#define ATA_IDENT_CYLINDERS    0x02 / 2
#define ATA_IDENT_HEADS        0x06 / 2
#define ATA_IDENT_SECTORS      0x0C / 2
#define ATA_IDENT_SERIAL       0x14 / 2
#define ATA_IDENT_MODEL        0x36 / 2
#define ATA_IDENT_CAPABILITIES 0x62 / 2
#define ATA_IDENT_FIELDVALID   0x6A / 2
#define ATA_IDENT_MAX_LBA      0x78 / 2
#define ATA_IDENT_COMMANDSETS  0xA4 / 2
#define ATA_IDENT_MAX_LBA_EXT  0xC8 / 2

#define IDE_ATA        0x00
#define IDE_ATAPI      0x01
 
#define ATA_MASTER     0x00
#define ATA_SLAVE      0x01

#define ATA_REG_DATA       0x00
#define ATA_REG_ERROR      0x01
#define ATA_REG_FEATURES   0x01
#define ATA_REG_SECCOUNT0  0x02
#define ATA_REG_LBA0       0x03
#define ATA_REG_LBA1       0x04
#define ATA_REG_LBA2       0x05
#define ATA_REG_HDDEVSEL   0x06
#define ATA_REG_COMMAND    0x07
#define ATA_REG_STATUS     0x07
#define ATA_REG_SECCOUNT1  0x08
#define ATA_REG_LBA3       0x09
#define ATA_REG_LBA4       0x0A
#define ATA_REG_LBA5       0x0B
#define ATA_REG_CONTROL    0x0C
#define ATA_REG_ALTSTATUS  0x0C
#define ATA_REG_DEVADDRESS 0x0D

// Channels:
#define      ATA_PRIMARY      0x00
#define      ATA_SECONDARY    0x01
 
// Directions:
#define      ATA_READ      0x00
#define      ATA_WRITE     0x013

typedef struct {
	uint8_t drive;
} ide_private_data;

typedef struct
{
	uint16_t general_config;       // Word 0: General configuration (bit flags)
    uint16_t cylinders;            // Word 1: Number of logical cylinders (CHS)
    uint16_t reserved_2;
    uint16_t heads;                // Word 3: Number of logical heads (CHS)
    uint16_t reserved_4;
    uint16_t sectors_per_track;     // Word 6: Number of logical sectors per track (CHS)
    uint16_t reserved_7_9[3];
    char serial_number[20];         // Words 10-19: Serial number (ASCII)
    uint16_t reserved_20_22[3];
    char firmware_revision[8];      // Words 23-26: Firmware revision (ASCII)
    char model_number[40];          // Words 27-46: Model number (ASCII)
    uint16_t maximum_rw_multiple;   // Word 47: Maximum sectors per R/W multiple
    uint16_t reserved_48;
    uint32_t capabilities;       // Words 49-50: Capabilities flags
    uint32_t reserved_51_52;
    uint16_t field_validity;        // Word 53: Field validity flags
    uint16_t current_cylinders;     // Word 54: Current CHS cylinders
    uint16_t current_heads;         // Word 55: Current CHS heads
    uint16_t current_sectors;       // Word 56: Current CHS sectors
    uint32_t current_capacity;   // Words 57-58: Current CHS capacity
    uint32_t lba_sector_count;   // Words 60-61: Total LBA sectors (28-bit)
    uint16_t multiword_dma;         // Word 63: Multiword DMA modes supported
    uint16_t pio_modes;             // Word 64: PIO modes supported
    uint16_t min_dma_cycle_time;    // Word 65: Minimum DMA cycle time
    uint16_t recommended_dma_cycle; // Word 66: Recommended DMA cycle time
    uint16_t min_pio_cycle_time;    // Word 67: Minimum PIO cycle time (without flow control)
    uint16_t min_pio_cycle_time_iordy; // Word 68: Minimum PIO cycle time (with IORDY flow control)
    uint16_t reserved_69_74[6];
    uint16_t queue_depth;           // Word 75: Queue depth (NCQ support)
    uint16_t sata_capabilities;     // Word 76: SATA capabilities
    uint16_t sata_features_supported; // Word 77: SATA features supported
    uint32_t reserved_78_79;
    uint16_t major_version;         // Word 80: ATA Major version number
    uint16_t minor_version;         // Word 81: ATA Minor version number
    uint16_t command_sets_supported[3]; // Words 82-84: Supported command sets (bitmask)
    uint16_t command_sets_enabled[3];   // Words 85-87: Enabled command sets (bitmask)
    uint16_t ultra_dma_modes;       // Word 88: Ultra DMA mode support
    uint16_t reserved_89_99[11];
    uint32_t lba48_sector_count;    // Words 100-103: 48-bit LBA sector count
    uint16_t reserved_104_126[23];
    uint16_t security_status;       // Word 128: Security status (drive locked, frozen, etc.)
    uint16_t reserved_129_159[31];
    uint16_t apm_value;             // Word 160: Advanced Power Management (APM)
    uint16_t reserved_161_191[31];
    uint16_t aam_value;             // Word 192: Acoustic Management (AAM)
    uint16_t reserved_193_254[62];
    uint16_t integrity_word;        // Word 255: Integrity checksum
} __attribute__((packed)) ATA_Identify_t;


void ATA_init();
void ATA_identify(ATA_Identify_t* buffer);