#pragma once

#include <stdint.h>
#include <stdbool.h>

extern bool ide_0x1F0_controller_present;
extern bool ide_0x170_controller_present;
extern uint8_t *ide_buf;

#define ATA_SR_BSY     0x80    // Busy
#define ATA_SR_DRDY    0x40    // Drive ready
#define ATA_SR_DF      0x20    // Drive write fault
#define ATA_SR_DSC     0x10    // Drive seek complete
#define ATA_SR_DRQ     0x08    // Data request ready
#define ATA_SR_CORR    0x04    // Corrected data
#define ATA_SR_IDX     0x02    // Index
#define ATA_SR_ERR     0x01    // Error

#define ATA_ER_BBK      0x80    // Bad block
#define ATA_ER_UNC      0x40    // Uncorrectable data
#define ATA_ER_MC       0x20    // Media changed
#define ATA_ER_IDNF     0x10    // ID mark not found
#define ATA_ER_MCR      0x08    // Media change request
#define ATA_ER_ABRT     0x04    // Command aborted
#define ATA_ER_TK0NF    0x02    // Track 0 not found
#define ATA_ER_AMNF     0x01    // No address mark

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

#define ATA_IDENT_DEVICETYPE   0
#define ATA_IDENT_CYLINDERS    2
#define ATA_IDENT_HEADS        6
#define ATA_IDENT_SECTORS      12
#define ATA_IDENT_SERIAL       20
#define ATA_IDENT_MODEL        54
#define ATA_IDENT_CAPABILITIES 98
#define ATA_IDENT_FIELDVALID   106
#define ATA_IDENT_MAX_LBA      120
#define ATA_IDENT_COMMANDSETS  164
#define ATA_IDENT_MAX_LBA_EXT  200

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


#define AHCI_ATA 0
#define AHCI_ATAPI 1

// Directions:
#define      ATA_READ      0x00
#define      ATA_WRITE     0x01

#define STATUS_TRUE 1
#define STATUS_FALSE 0

typedef struct
{
    uint16_t base;  // I/O Base.
    uint16_t ctrl;  // Control Base
    uint16_t bmide; // Bus Master IDE
    uint8_t nIEN;   // nIEN (No Interrupt);
} IDEChannelRegisters;

typedef struct ide_device
{
    uint8_t Reserved;      // 0 (Empty) or 1 (This Drive really exists).
    uint8_t Channel;       // 0 (Primary Channel) or 1 (Secondary Channel).
    uint8_t Drive;         // 0 (Master Drive) or 1 (Slave Drive).
    uint16_t Type;         // 0: ATA, 1:ATAPI.
    uint16_t Signature;    // Drive Signature
    uint16_t Capabilities; // Features.
    uint32_t CommandSets;    // Command Sets Supported.
    uint32_t Size;           // Size in Sectors.
    char Model[41];     // Model in string.
} ide_device;

typedef struct
{
    uint16_t drive;
} ide_private_data;

extern ide_device ide_devices[4];
extern IDEChannelRegisters channels[2];
extern uint8_t ide_devices_count;

void ide_write(uint8_t channel, uint8_t reg, uint8_t data);
uint8_t ide_read(uint8_t channel, uint8_t reg);
void ide_read_buffer(uint8_t channel, uint8_t reg, uint32_t buffer, uint32_t quads);

void ideSelectDrive(uint8_t channel, uint8_t drive);

uint8_t ide_print_error(uint32_t drive, uint8_t err);

uint8_t ide_is_bus_floating(uint16_t base_port);

void ide_400ns_delay(uint8_t channel);
uint8_t ide_polling(uint8_t channel, uint32_t advanced_check);
uint8_t ide_print_error(uint32_t drive, uint8_t err);
bool ide_initialize(uint32_t BAR0, uint32_t BAR1, uint32_t BAR2, uint32_t BAR3, uint32_t BAR4);

void __attribute__((cdecl)) SendData(uint32_t bus, uint16_t selector, uint32_t words, uint32_t edi);
void __attribute__((cdecl)) ReceiveData(uint32_t bus, uint16_t selector, uint32_t words, uint32_t edi);
void __attribute__((cdecl)) SendPacket(uint32_t bus, uint8_t atapi_packet[12]);
void __attribute__((cdecl)) insl(int port, uint32_t addr, int cnt);
