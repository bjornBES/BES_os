#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "fs/disk.h"
#include "drivers/device.h"

typedef int fd_t;

#define VFS_FD_STDIN (fd_t)0
#define VFS_FD_STDOUT (fd_t)1
#define VFS_FD_STDERR (fd_t)2
#define VFS_FD_DEBUG (fd_t)3

int VFS_Write(fd_t file, uint8_t *data, size_t size);
bool VFS_Read(char* filename, uint8_t* buffer); 

typedef enum FS
{
    FS_FATFS,
} FS;
typedef enum CONNECTOR
{
    CONNECTOR_AHCI,
    CONNECTOR_DEV,
    CONNECTOR_SYS,
    CONNECTOR_PROC
} CONNECTOR;

typedef struct MountPoint_t
{
    char* loc;
    device_t *dev;
} MountPoint;

bool tryMountFS(device_t* dev, char* loc);

void VFS_init();

