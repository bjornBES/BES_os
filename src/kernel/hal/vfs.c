#include "vfs.h"
#include "string.h"
#include "malloc.h"
#include "memory.h"
#include "fs/devfs/devfs.h"
#include "fs/ext2/ext2.h"
#include "fs/fat32/fat32.h"
#include "proc.h"

#include <arch/i686/vga_text.h>
#include <arch/i686/e9.h>
#include <stdio.h>

#define MAX_MOUNTS 16

MountPoint** mountPoints = 0;
int lastMountId = 0;

int VFS_Write(fd_t file, uint8_t* data, size_t size)
{
    switch (file)
    {
    case VFS_FD_STDIN:
        return 0;
    case VFS_FD_STDOUT:
    case VFS_FD_STDERR:
        for (size_t i = 0; i < size; i++)
            VGA_putc(data[i]);
        return size;

    case VFS_FD_DEBUG:
        for (size_t i = 0; i < size; i++)
            e9_putc(data[i]);
        return size;

    default:
        return -1;
    }
}

device_t *checkMountPoint(char* loc)
{
	for (size_t i = 0; i < lastMountId; i++)
	{
		if (strcmp(loc, mountPoints[i]->loc) == 0)
		{
			return mountPoints[i]->dev;
		}
	}
	return NULL;
}

bool tryMountFS(device_t* dev, char* loc)
{
	if(!dev || (dev->id))
	{
		return false;
	}
	if (checkMountPoint(loc))
	{
		return false;
	}

	if (fat32Probe(dev))
	{

	}
}

void VFS_init()
{
	printf("Loading VFS\n");
	mountPoints = (MountPoint **)malloc(sizeof(MountPoint) * MAX_MOUNTS);
}