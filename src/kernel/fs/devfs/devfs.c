#include <stdint.h>

#include "stdio.h"
#include "string.h"
#include "hal/vfs.h"
#include "hal/hal.h"
#include "memory.h"
#include "malloc.h"

#include "devfs.h"

#define MODULE                  "DEVFS"

#define UNUSED __attribute__((unused))

uint8_t devfs_read()
{
	return 0;
}

uint8_t devfs_read_dir()
{
	printf(".\n..\n");
}

uint8_t devfs_exist(char *f)
{
	if(strcmp(f, "/") == 0)
		return 1;
	return 0;
}


void devfs_init()
{
	/*
	filesystem_t *fs = (filesystem_t *)malloc(sizeof(filesystem_t));
	printf("Mounting /dev filesystem.\n");
	fs->name = "DEVFS";
	fs->probe = devfs_probe;
	fs->read = devfs_read;
	fs->read_dir = devfs_read_dir;
	fs->exist = devfs_exist;
	fs->mount = devfs_mount;
	device_t *dev_dev = (device_t *)malloc(sizeof(device_t));
	dev_dev->name = "DEVDEV";
	dev_dev->unique_id = 11;
	dev_dev->dev_type = DEVICE_BLOCK;
	dev_dev->fs = fs;
	dev_dev->read = 0;
	dev_dev->write = 0;
	device_add(dev_dev);
	dev_dev = device_get_by_id(11);
	device_try_to_mount(dev_dev, "/dev/");
	*/
}