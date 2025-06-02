#pragma once
#include "defaultInclude.h"
#include "fs/disk.h"
#include "drivers/device.h"

typedef int fd_t;

#define VFS_FD_STDIN (fd_t) ~1
#define VFS_FD_STDOUT (fd_t) ~2
#define VFS_FD_STDERR (fd_t) ~3
#define VFS_FD_DEBUG (fd_t) ~4

#define MAX_PATH_SIZE           256
#define MAX_FILE_HANDLES        10
#define ROOT_DIRECTORY_HANDLE   -1

int VFS_Write(fd_t file, uint8_t *data, size_t size);
int VFS_Read(fd_t file, void *buffer, size_t size);
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

typedef enum {
    FS_FILE,
    FS_DIRECTORY
} file_type_t;

typedef struct vfs_node
{
    char name[MAX_PATH_SIZE];
    file_type_t type;
    uint32_t size;
    uint32_t inode;

    uint32_t mountPoint;

    void *fs_specific_data;
} vfs_node_t;

typedef struct MountPoint_t
{
    char *loc;
    device_t *dev;
} MountPoint;

fd_t VFS_Open(char* path);
void VFS_close(vfs_node_t *node);
// vfs_node_t *VFS_finddir(fd_t file, void* buffer, uint);
// vfs_node_t *VFS_readdir(fd_t file, void* buffer, uint);

bool tryMountFS(device_t *dev, char *loc);

void VFS_init();
