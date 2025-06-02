#include "vfs.h"
#include "string.h"
#include "malloc.h"
#include "memory.h"
#include "fs/devfs/devfs.h"
// #include "fs/ext2/ext2.h"
#include "fs/fat32/fat32.h"
#include "proc.h"
#include "debug.h"

#include <drivers/VGA/vga.h>
#include <arch/i686/e9.h>
#include <stdio.h>

#define MAX_MOUNTS 16
#define MODULE "VFS"

typedef struct
{
	vfs_node_t *node;
	uint32_t offset;
} file_descriptor_t;

file_descriptor_t fd_table[MAX_FILE_HANDLES];

Page *mountPointsPage;
MountPoint **mountPoints = 0;
Page **mPages;
Page *mPagesPage;
int lastMountId = 0;

vfs_node_t *vfs_root;

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
		if (file < 0 || file >= MAX_FILE_HANDLES || &fd_table[file] == NULL)
		{
			return -1;
		}
		// file_descriptor_t *node = &fd_table[file];
		// return node->write(node, 0, size, buffer);
	}
	return -1;
}

int Sys_Read(file_descriptor_t *file, void *buffer, size_t size)
{
	if (file == NULL || buffer == NULL || size == 0)
	{
		return -1;
	}

	vfs_node_t *node = file->node;
	if (node == NULL)
	{
		return -1; // Invalid file descriptor
	}

	if (node->type != FS_FILE)
	{
		return -1; // Not a file
	}

	MountPoint*	mountpoint = mountPoints[node->mountPoint];
	if (mountpoint == NULL)
	{
		return -1; // Mount point not found
	}
	filesystemInfo_t *fs = mountpoint->dev->fs;
	if (fs == NULL)
	{
		return -1; // Filesystem not mounted or read function not defined
	}

	if (fs->read == NULL)
	{
		return -1; // No read function defined
	}

	return fs->read(node->name, buffer, mountpoint->dev, fs->priv_data);
}

int VFS_Read(fd_t file, void *buffer, size_t size)
{
	switch (file)
	{
	case VFS_FD_STDIN:
		return 0;
	case VFS_FD_STDOUT:
	case VFS_FD_STDERR:
		return 0;
	case VFS_FD_DEBUG:
		return 0;

	default:
		if (file < 0 || file >= MAX_FILE_HANDLES || &fd_table[file] == NULL)
		{
			return -1;
		}
		file_descriptor_t *node = &fd_table[file];
		return Sys_Read(node, buffer, size);
	}
	return -1;
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
	if (FAT_Probe(dev))
	{
		log_debug(MODULE, "FAT32 probed successly");
		if (FAT_Mount(dev, dev->fs->priv_data))
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

vfs_node_t *VFS_findDir(vfs_node_t *current, char *token)
{
	log_debug(MODULE, "VFS_findDir: current = %p, token = %s", current, token);
	if (!current || !token)
	{
		return NULL;
	}
	MountPoint* mountpoint = mountPoints[current->mountPoint];
	if (!mountpoint)
	{
		return NULL;
	}

	filesystemInfo_t *fs = mountpoint->dev->fs;


	return NULL;
}

vfs_node_t *vfs_resolve_path(char *path)
{
	log_debug(MODULE, "vfs_resolve_path: path = %s", path);
	// path == "/dev/test.txt"
	if (!path || path[0] != '/')
		return NULL;

	vfs_node_t *current = vfs_root;
	char *token;
	char path_copy[256];
	strcpy(path_copy, path);

	// path_copy = "/dev/test.txt"
	token = strtok(path_copy, "/");
	// token = dev
	while (token && current)
	{
		current = VFS_findDir(current, token);
		token = strtok(NULL, "/");
	}

	return current;
}

int __find_mount(char *filename, int *adjust)
{
	log_debug(MODULE, "filename %s string length %u", filename, strlen(filename) + 1);
	Page origPage;
	char *orig = (char *)calloc(1, strlen(filename) + 1, &origPage);
	memcpy(orig, filename, strlen(filename) + 1);
	if (orig[strlen(orig)] == '/')
	{
		str_backspace(orig, '/');
	}
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

fd_t VFS_Open(char *path)
{
	vfs_node_t *node = vfs_resolve_path(path);
	return 0;
}

void VFS_init()
{
	printf("Loading VFS\n");
	mountPoints = (MountPoint **)malloc(sizeof(MountPoint) * MAX_MOUNTS, mountPointsPage);
	mPages = (Page **)malloc(sizeof(Page) * MAX_MOUNTS, mPagesPage);
}