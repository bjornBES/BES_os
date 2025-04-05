#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "drivers/device.h"

#define HBA_DET_PRESENT 	3
#define HBA_IPM_ACTIVE 		1
#define SATA_READ_DMA_EX 	0x25
#define SATA_WRITE_DMA_EX 	0x35
#define SATA_IDENTIFY_DEVICE 0xEC
#define HBA_CMD_CR 			(1 << 15)
#define HBA_CMD_FR 			(1 << 14)
#define HBA_CMD_FRE 		(1 << 4)
#define HBA_CMD_SUD 		(1 << 1)
#define HBA_CMD_ST 			(1)
#define SATA_BUSY 			0x80
#define SATA_DRQ 			0x08

#define HBA_PxCMD_ST    0x0001
#define HBA_PxCMD_FRE   0x0010
#define HBA_PxCMD_FR    0x4000
#define HBA_PxCMD_CR    0x8000

typedef enum
{
	FIS_TYPE_REG_H2D = 0x27,   // Register FIS - host to device
	FIS_TYPE_REG_D2H = 0x34,   // Register FIS - device to host
	FIS_TYPE_DMA_ACT = 0x39,   // DMA activate FIS - device to host
	FIS_TYPE_DMA_SETUP = 0x41, // DMA setup FIS - bidirectional
	FIS_TYPE_DATA = 0x46,	   // Data FIS - bidirectional
	FIS_TYPE_BIST = 0x58,	   // BIST activate FIS - bidirectional
	FIS_TYPE_PIO_SETUP = 0x5F, // PIO setup FIS - device to host
	FIS_TYPE_DEV_BITS = 0xA1,  // Set device bits FIS - device to host
} FIS_TYPE;

typedef struct tagHBA_PORT
{
	uint32_t clb;		// 0x00, command list base address, 1K-byte aligned
	uint32_t clbu;		// 0x04, command list base address upper 32 bits
	uint32_t fb;		// 0x08, FIS base address, 256-byte aligned
	uint32_t fbu;		// 0x0C, FIS base address upper 32 bits
	uint32_t is;		// 0x10, interrupt status
	uint32_t ie;		// 0x14, interrupt enable
	uint32_t cmd;		// 0x18, command and status
	uint32_t rsv0;		// 0x1C, Reserved
	uint32_t tfd;		// 0x20, task file data
	uint32_t sig;		// 0x24, signature
	uint32_t ssts;		// 0x28, SATA status (SCR0:SStatus)
	uint32_t sctl;		// 0x2C, SATA control (SCR2:SControl)
	uint32_t serr;		// 0x30, SATA error (SCR1:SError)
	uint32_t sact;		// 0x34, SATA active (SCR3:SActive)
	uint32_t ci;		// 0x38, command issue
	uint32_t sntf;		// 0x3C, SATA notification (SCR4:SNotification)
	uint32_t fbs;		// 0x40, FIS-based switch control
	uint32_t rsv1[11];	// 0x44 ~ 0x6F, Reserved
	uint32_t vendor[4]; // 0x70 ~ 0x7F, vendor specific
} HBAPort;

typedef struct tagHBA_MEM
{
	// 0x00 - 0x2B, Generic Host Control
	uint32_t cap;	  // 0x00: Host Capabilities
	uint32_t ghc;	  // 0x04: Global HBA Control
	uint32_t is;	  // 0x08: Interrupt Status
	uint32_t pi;	  // 0x0C: Port Implemented
	uint32_t version; // 0x10: Version
	uint32_t ccc_ctl; // 0x14: Command Completion Coalescing Control
	uint32_t ccc_pts; // 0x18: Command Completion Coalescing Ports
	uint32_t em_loc;  // 0x1C: Enclosure Management Location
	uint32_t em_ctl;  // 0x20: Enclosure Management Control
	uint32_t cap2;	  // 0x24: Extended Capabilities
	uint32_t bohc;	  // 0x28: BIOS/OS Handoff Control
	uint8_t reserved[116];

	// 0xA0 - 0xFF, Vendor specific registers
	uint8_t vendor[96];

	// 0x100 - 0x10FF, Port control registers
	HBAPort ports[1]; // 1 ~ 32
} HBAData;

typedef struct tagFIS_REG_H2D
{
	// DWORD 0
	uint8_t fis_type; // FIS_TYPE_REG_H2D

	uint8_t pmport : 4; // Port multiplier
	uint8_t rsv0 : 3;	// Reserved
	uint8_t c : 1;		// 1: Command, 0: Control

	uint8_t command;  // Command register
	uint8_t featurel; // Feature register, 7:0

	// DWORD 1
	uint8_t lba0;	// LBA low register, 7:0
	uint8_t lba1;	// LBA mid register, 15:8
	uint8_t lba2;	// LBA high register, 23:16
	uint8_t device; // Device register

	// DWORD 2
	uint8_t lba3;	  // LBA register, 31:24
	uint8_t lba4;	  // LBA register, 39:32
	uint8_t lba5;	  // LBA register, 47:40
	uint8_t featureh; // Feature register, 15:8

	// DWORD 3
	uint8_t countl;	 // Count register, 7:0
	uint8_t counth;	 // Count register, 15:8
	uint8_t icc;	 // Isochronous command completion
	uint8_t control; // Control register

	// DWORD 4
	uint32_t rsv1; // Reserved
} FIS_REG_H2D;

typedef struct tagFIS_D2H
{
	// DWORD 0
	uint8_t fis_type; // FIS_TYPE_REG_D2H

	uint8_t pmport : 4; // Port multiplier
	uint8_t rsv0 : 2;	// Reserved
	uint8_t i : 1;		// Interrupt bit
	uint8_t rsv1 : 1;	// Reserved

	uint8_t status; // Status register
	uint8_t error;	// Error register

	// DWORD 1
	uint8_t lba0;	// LBA low register, 7:0
	uint8_t lba1;	// LBA mid register, 15:8
	uint8_t lba2;	// LBA high register, 23:16
	uint8_t device; // Device register

	// DWORD 2
	uint8_t lba3; // LBA register, 31:24
	uint8_t lba4; // LBA register, 39:32
	uint8_t lba5; // LBA register, 47:40
	uint8_t rsv2; // Reserved

	// DWORD 3
	uint8_t countl;	 // Count register, 7:0
	uint8_t counth;	 // Count register, 15:8
	uint8_t rsv3[2]; // Reserved

	// DWORD 4
	uint32_t rsv4; // Reserved
} FIS_D2H;

typedef struct tagFIS_DATA
{
	// DWORD 0
	uint8_t fis_type; // FIS_TYPE_DATA

	uint8_t pmport : 4; // Port multiplier
	uint8_t rsv0 : 4;	// Reserved

	uint8_t rsv1[2]; // Reserved

	// DWORD 1 ~ N
	uint32_t data[1]; // Payload
} FIS_DATA;

typedef struct tagFIS_PIO_SETUP
{
	// DWORD 0
	uint8_t fis_type; // FIS_TYPE_PIO_SETUP

	uint8_t pmport : 4; // Port multiplier
	uint8_t rsv0 : 1;	// Reserved
	uint8_t d : 1;		// Data transfer direction, 1 - device to host
	uint8_t i : 1;		// Interrupt bit
	uint8_t rsv1 : 1;

	uint8_t status; // Status register
	uint8_t error;	// Error register

	// DWORD 1
	uint8_t lba0;	// LBA low register, 7:0
	uint8_t lba1;	// LBA mid register, 15:8
	uint8_t lba2;	// LBA high register, 23:16
	uint8_t device; // Device register

	// DWORD 2
	uint8_t lba3; // LBA register, 31:24
	uint8_t lba4; // LBA register, 39:32
	uint8_t lba5; // LBA register, 47:40
	uint8_t rsv2; // Reserved

	// DWORD 3
	uint8_t countl;	  // Count register, 7:0
	uint8_t counth;	  // Count register, 15:8
	uint8_t rsv3;	  // Reserved
	uint8_t e_status; // New value of status register

	// DWORD 4
	uint16_t tc;	 // Transfer count
	uint8_t rsv4[2]; // Reserved
} FIS_PIO_SETUP;

typedef struct tagFIS_DMA_SETUP
{
	// DWORD 0
	uint8_t fis_type; // FIS_TYPE_DMA_SETUP

	uint8_t pmport : 4; // Port multiplier
	uint8_t rsv0 : 1;	// Reserved
	uint8_t d : 1;		// Data transfer direction, 1 - device to host
	uint8_t i : 1;		// Interrupt bit
	uint8_t a : 1;		// Auto-activate. Specifies if DMA Activate FIS is needed

	uint16_t rsved; // Reserved

	// DWORD 1&2

	uint64_t DMAbufferID; // DMA Buffer Identifier. Used to Identify DMA buffer in host memory.
						  // SATA Spec says host specific and not in Spec. Trying AHCI spec might work.

	// DWORD 3
	uint32_t rsvd; // More reserved

	// DWORD 4
	uint32_t DMAbufOffset; // Byte offset into buffer. First 2 bits must be 0

	// DWORD 5
	uint32_t TransferCount; // Number of bytes to transfer. Bit 0 must be 0

	// DWORD 6
	uint32_t resvd; // Reserved

} FIS_DMA_SETUP;

typedef struct tagHBA_FIS
{
	FIS_DMA_SETUP DMASetup;
	uint8_t pad0[4];

	FIS_PIO_SETUP PIOSetup;
	uint8_t pad1[12];

	FIS_D2H RegDeviceToHost;
	uint8_t pad2[4];

	uint16_t dev_bits;

	uint8_t ufis[64];

	uint8_t reserved[96];
} HBA_FIS;

typedef struct tagHBA_CMD_HEADER
{
	uint8_t cfl : 5;
	uint8_t a : 1;
	uint8_t w : 1;
	uint8_t p : 1;
	uint8_t r : 1;
	uint8_t b : 1;
	uint8_t c : 1;
	uint8_t reserved0 : 1;
	uint8_t pmp : 4;

	uint16_t prdtl;

	volatile uint32_t prdbc;

	uint32_t ctba;
	uint32_t ctbau;

	uint32_t reserved1[4];
} HBA_CMD_HEADER;

typedef struct tagHBA_PRDT_ENTRY
{
	uint32_t dba;  // Data base address
	uint32_t dbau; // Data base address upper 32 bits
	uint32_t rsv0; // Reserved

	// DW3
	uint32_t dbc : 22; // Byte count, 4M max
	uint32_t rsv1 : 9; // Reserved
	uint32_t i : 1;	   // Interrupt on completion
} HBA_PRDT_ENTRY;

typedef struct tagHBA_CMD_TBL
{
	// 0x00
	uint8_t cfis[64]; // Command FIS

	// 0x40
	uint8_t acmd[16]; // ATAPI command, 12 or 16 bytes

	// 0x50
	uint8_t rsv[48]; // Reserved

	// 0x80
	HBA_PRDT_ENTRY prdt_entry[1]; // Physical region descriptor table entries, 0 ~ 65535
} HBA_CMD_TBL;

typedef struct
{
	uint16_t gen_cfg;
	uint16_t unused0[9];
	char serial_number[20];
	uint16_t unused1[3];
	char firmware_rev[8];
	char model_number[40];
	uint16_t sectors_per_int;
	uint16_t reserved0;
	uint16_t capabilities[2];
	uint16_t unused2[2];
	uint16_t valid_values;
	uint16_t unused3[5];
	uint16_t multi_sector;
	uint32_t user_addressable_sectors;
	uint16_t unused4[13];
	uint16_t max_queue_depth;
	uint64_t reserved1;
	uint16_t major_version;
	uint16_t minor_version;
	uint32_t command_sets_supported;
	uint16_t command_extension_supported;
	uint32_t command_sets_enabled;
	uint16_t command_set_default;
	uint16_t ultra_dma;
	uint16_t security_erase_time;
	uint16_t e_security_erase_time;
	uint16_t power_mgmt_val;
	uint16_t master_password_rev;
	uint16_t hw_reset_result;
	uint16_t acoustic_mgmt;
	uint16_t streaming[5];
	uint64_t total_sectors;
	uint32_t unused5;
	uint16_t logical_sector_size;
	uint16_t unused6[10];
	uint32_t words_per_logical_sector;
	uint16_t unused7[136];
	uint16_t checksum;
} __attribute__((packed)) sata_identify_packet;

uint8_t ahci_read_sectors(void *buf, uint64_t start_sector, uint32_t count, device_t* device);
void AHCI_init(uint32_t bar5);
