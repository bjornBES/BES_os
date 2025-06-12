#include <stdint.h>
#include <stdbool.h>

#include "hal/vfs.h"


#pragma once

#define SECTOR_SIZE             512
#define FAT_CACHE_SIZE          5
#define DIR_ENTRY_SIZE          32
#define FAT32_EOC               0x0FFFFFF8 // End of Cluster marker for FAT32

typedef struct 
{
    uint8_t Name[11];
    uint8_t Attributes;
    uint8_t _Reserved;
    uint8_t CreatedTimeTenths;
    uint16_t CreatedTime;
    uint16_t CreatedDate;
    uint16_t AccessedDate;
    uint16_t FirstClusterHigh;
    uint16_t ModifiedTime;
    uint16_t ModifiedDate;
    uint16_t FirstClusterLow;
    uint32_t Size;
} __attribute__((packed)) FAT_DirectoryEntry;

typedef struct 
{
    uint8_t Order;
    int16_t name1[5];
    uint8_t Attribute;
    uint8_t LongEntryType;
    uint8_t Checksum;
    int16_t name2[6];
    uint16_t _AlwaysZero;
    int16_t name3[2];
} __attribute__((packed)) FAT_LongFileEntry;

typedef union
{
    FAT_DirectoryEntry Entry;
    FAT_LongFileEntry LongEntry;
} FAT_FileEntry;


#define FAT_LFN_LAST            0x40
                  
// not in use
typedef struct 
{
    int Handle;
    bool IsDirectory;
    uint32_t Position;
    uint32_t Size;
} FAT_File;

typedef struct 
{
    // extended boot record
    uint8_t DriveNumber;
    uint8_t _Reserved;
    uint8_t Signature;
    uint32_t VolumeId;          // serial number, value doesn't matter
    uint8_t VolumeLabel[11];    // 11 bytes, padded with spaces
    uint8_t SystemId[8];
} __attribute__((packed)) FAT_ExtendedBootRecord;

typedef struct 
{
    uint32_t SectorsPerFat;
    uint16_t Flags;
    uint16_t FatVersion;
    uint32_t RootDirectoryCluster;
    uint16_t FSInfoSector;
    uint16_t BackupBootSector;
    uint8_t _Reserved[12];
    FAT_ExtendedBootRecord EBR;

} __attribute((packed)) FAT32_ExtendedBootRecord;

typedef struct 
{
    uint8_t BootJumpInstruction[3];
    uint8_t OemIdentifier[8];
    uint16_t BytesPerSector;
    uint8_t SectorsPerCluster;
    uint16_t ReservedSectors;
    uint8_t FatCount;
    uint16_t DirEntryCount;
    uint16_t TotalSectors;
    uint8_t MediaDescriptorType;
    uint16_t SectorsPerFat;
    uint16_t SectorsPerTrack;
    uint16_t Heads;
    uint32_t HiddenSectors;
    uint32_t LargeSectorCount;

    union {
        FAT_ExtendedBootRecord EBR1216;
        FAT32_ExtendedBootRecord EBR32;
    };

    // ... we don't care about code ...

} __attribute__((packed)) FAT_BootSector;

// not in use
typedef struct
{
    FAT_File Public;
    bool Opened;
    uint32_t FirstCluster;
    uint32_t CurrentCluster;
    uint32_t CurrentSectorInCluster;
    uint8_t* Buffer;

} FAT_FileData;

typedef struct {
    uint8_t Order;
    int16_t Chars[13];
} FAT_LFNBlock;

typedef struct 
{
    FAT_FileEntry* entries;  // Dynamically allocated based on root directory size
    uint32_t entryCount;     // Track number of entries
} FAT_Directory; 

typedef enum __FatType
{
    FAT12,
    FAT16,
    FAT32,
} FatType;

typedef struct
{
    union
    {
        FAT_BootSector BootSector;
        uint8_t BootSectorBytes[SECTOR_SIZE];
    } BS;

    FatType FATType;
    uint32_t FirstDataSector;
    uint32_t RootDirSector;
    uint32_t CountofClusters;
    uint32_t FATSector;
    uint32_t FATSize;

    FAT_Directory RootDirectory;

    FAT_LFNBlock* LFNBlocks;
    int LFNCount;

    // this is where the last read was at
    uint8_t FatCache[FAT_CACHE_SIZE * SECTOR_SIZE];
    uint32_t FatCachePosition;

} FAT_Data;

typedef struct __fat_priv_data {
    uint32_t BytesPerCluster;
    uint32_t ClusterSize;
    uint32_t RootDirSizeSec;
} __attribute__((packed)) fatPrivData;

enum FAT_Attributes
{
    FAT_ATTRIBUTE_READ_ONLY         = 0x01,
    FAT_ATTRIBUTE_HIDDEN            = 0x02,
    FAT_ATTRIBUTE_SYSTEM            = 0x04,
    FAT_ATTRIBUTE_VOLUME_ID         = 0x08,
    FAT_ATTRIBUTE_DIRECTORY         = 0x10,
    FAT_ATTRIBUTE_ARCHIVE           = 0x20,
    FAT_ATTRIBUTE_LFN               = FAT_ATTRIBUTE_READ_ONLY | FAT_ATTRIBUTE_HIDDEN | FAT_ATTRIBUTE_SYSTEM | FAT_ATTRIBUTE_VOLUME_ID
};

/// @brief 
/// @param shortName is the 8.3 format 
/// @param name is the a normal format
void GetName(char* shortName, char *name);

bool FAT_Probe(device_t* dev);
bool FAT_Mount(device_t *dev, void *priv);
bool FAT_GetRoot(void* node, device_t* dev, void *priv);