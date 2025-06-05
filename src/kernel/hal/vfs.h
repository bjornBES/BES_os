#pragma once
#include "defaultInclude.h"
#include "fs/disk.h"
#include "drivers/device.h"

typedef int fd_t;

#define MAX_PATH_SIZE           256
#define MAX_FILE_HANDLES        10
#define ROOT_DIRECTORY_HANDLE   -1
#define VFS_FD_START (fd_t) 4 // Start from 4 to avoid stdin, stdout, stderr, debug

#define VFS_FD_STDIN (fd_t) 0
#define VFS_FD_STDOUT (fd_t) 1
#define VFS_FD_STDERR (fd_t) 2
#define VFS_FD_DEBUG (fd_t) 3

#define VFS_INVALID_FD (fd_t) -1



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

#define VFS_FILE 0x01
#define VFS_DIR  0x02
#define VFS_SYMLINK 0x04
#define VFS_CHARDEV 0x08
#define VFS_BLOCKDEV 0x10
#define VFS_MOUNTPOINT 0x20
#define VFS_READABLE 0x40
#define VFS_WRITABLE 0x80
typedef struct vfs_node
{
    char name[MAX_PATH_SIZE];
    uint32_t size;
    uint32_t inode;
    uint8_t permissions; // 0x1 = read, 0x2 = write, 0x4 = execute
    uint32_t mountingPointId; // ID of the mount point this node belongs to 
} vfs_node_t;

typedef struct MountPoint_t
{
    char *loc;
    device_t *dev;
    vfs_node_t *root_node;
} MountPoint;

bool VFS_Seek(fd_t file, uint64_t offset);
int VFS_GetOffset(fd_t file);
int VFS_GetSize(fd_t file);

fd_t VFS_Open(char* path);
bool VFS_Close(fd_t file);
// vfs_node_t *VFS_finddir(fd_t file, void* buffer, uint);
// vfs_node_t *VFS_readdir(fd_t file, void* buffer, uint);

vfs_node_t *VFS_GetNode(fd_t file);

bool MountDevice(device_t *dev, char *loc);

void VFS_init();
