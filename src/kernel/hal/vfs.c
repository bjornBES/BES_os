#include "vfs.h"
#include "string.h"
#include "malloc.h"
#include "memory.h"
#include "fs/devfs/devfs.h"
// #include "fs/ext2/ext2.h"
#include "fs/fat32/fat32.h"
#include "proc.h"
#include "debug.h"

#include <arch/i686/vga_text.h>
#include <arch/i686/e9.h>
#include <stdio.h>

#define MAX_MOUNTS 16
#define MODULE "VFS"

Page* mountPointsPage;
MountPoint **mountPoints = 0;
Page** mPages;
Page* mPagesPage;
int lastMountId = 0;

int VFS_Write(fd_t file, uint8_t *data, size_t size)
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

device_t *checkMountPoint(char *loc)
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

bool tryMountFS(device_t *dev, char *loc)
{
	if (dev == NULL || (dev->id == 0))
	{
		log_debug(MODULE, "dev = %p dev.id = %u", dev, dev->id);
		return false;
	}
	log_debug(MODULE, "dev = %p", dev);
	if (checkMountPoint(loc))
	{
		return false;
	}
	log_debug(MODULE, "after cheaking Mounting Points");
	if (fatProbe(dev))
	{
		log_debug(MODULE, "FAT32 probed successly");
		if (fatMount(dev, dev->fs->priv_data))
		{
			MountPoint *m = (MountPoint *)malloc(sizeof(MountPoint), mPages[lastMountId]);
			m->loc = loc;
			m->dev = dev;
			lastMountId++;
			mountPoints[lastMountId - 1] = m;
			log_debug(MODULE, "good");
			return true;
		}
		return false;
	}
	return false;
}

int __find_mount(char *filename, int *adjust)
{
	log_debug(MODULE, "filename %s", filename);
	Page origPage;
	char *orig = (char *)malloc(strlen(filename) + 1, &origPage);
	memset(orig, 0, strlen(filename) + 1);
	memcpy(orig, filename, strlen(filename) + 1);
	if (orig[strlen(orig)] == '/')
	{
		str_backspace(orig, '/');
	}
	log_debug(MODULE, "filename %s", orig);
	while (1)
	{
		for (int i = 0; i < MAX_MOUNTS; i++)
		{
			if (!mountPoints[i])
				break;
			if (strcmp(mountPoints[i]->loc, orig) == 0)
			{
				/* Adjust the orig to make it relative to fs/dev */
				*adjust = (strlen(orig) - 1);
				/*
				 */
				log_debug(MODULE, "returning %s (%d) i=%d orig=%s adjust=%d\n", mountPoints[i]->dev->name, mountPoints[i]->dev->id, i, orig, *adjust);
				free(orig, &origPage);
				return i;
			}
		}
		if (strcmp(orig, "/") == 0)
		break;
		str_backspace(orig, '/');
	}
	free(orig, &origPage);
	return false;
}

bool VFS_Read(char *filename, uint8_t *buffer)
{
	int adjust = 0;
	int i = __find_mount(filename, &adjust);
	// log_debug(MODULE, "found mount %u", i);
	filename += adjust;
	// log_debug(MODULE, "Passing with adjust %d: %s\n", adjust, filename);
	int rc = mountPoints[i]->dev->fs->read(filename, buffer, mountPoints[i]->dev, mountPoints[i]->dev->fs->priv_data);
	return rc;
}

void VFS_init()
{
	printf("Loading VFS\n");
	mountPoints = (MountPoint **)malloc(sizeof(MountPoint) * MAX_MOUNTS, mountPointsPage);
	mPages = (Page **)malloc(sizeof(Page) * MAX_MOUNTS, mPagesPage);
}