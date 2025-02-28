#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "fs/disk.h"
#include "drivers/device.h"

typedef int fd_t;

#define VFS_FD_STDIN 0
#define VFS_FD_STDOUT 1
#define VFS_FD_STDERR 2
#define VFS_FD_DEBUG 3

int VFS_Write(fd_t file, uint8_t *data, size_t size);
int VFS_Read(fd_t file, uint8_t* buffer, size_t size); 

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


void VFS_init();

