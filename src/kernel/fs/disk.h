#pragma once

#include <stdint.h>
#include "drivers/ATA/ATA.h"


#define MBR_PARTITION_1 0x01BE
#define MBR_PARTITION_2 0x01CE
#define MBR_PARTITION_3 0x01DE
#define MBR_PARTITION_4 0x01EE

#define MBR_BOOTABLE 0x80
#define MBR_REGULAR 0x00

typedef struct {
    uint8_t  status;
    uint8_t  chs_first_sector[3];
    uint8_t  type;
    uint8_t  chs_last_sector[3];
    uint32_t lba_first_sector;
    uint32_t sector_count;
  } mbr_partition;

typedef struct {
    uint16_t cylinders;      // Number of cylinders (CHS mode)
    uint16_t heads;          // Number of heads (CHS mode)
    uint16_t sectors_per_track; // Sectors per track (CHS mode)

    uint32_t total_sectors_28;  // Total number of sectors (LBA28 mode)
    uint64_t total_sectors_48;  // Total number of sectors (LBA48 mode, for large disks)
    uint64_t disk_size_bytes;   // Disk size in bytes (computed)
} DISK;

void GetDisk(ATA_Identify_t* ATA_Identify, DISK* diskOut);

void DISK_LBA2CHS(DISK* disk, uint32_t lba, uint16_t* cylinderOut, uint16_t* sectorOut, uint16_t* headOut);
void DISK_CHS2LBA(DISK* disk, uint16_t cylinder, uint16_t sector, uint16_t head, uint32_t* lbaOut);
