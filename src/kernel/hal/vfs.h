#pragma once
#include <stdint.h>
#include <stddef.h>

#include "device.h"

struct __device_t;

typedef struct __fs_t {
	char *name;
	uint8_t (*probe)(struct __device_t *);
	uint8_t (*read)(char *, char *, struct __device_t *, void *);
	uint8_t (*read_dir)(char *, char *, struct __device_t *, void *);
	uint8_t (*touch)(char *fn, struct __device_t *, void *);
	uint8_t (*writefile)(char *fn, char *buf, uint32_t len, struct __device_t *, void *);
	uint8_t (*exist)(char *filename, struct __device_t *, void *);
	uint8_t (*mount)(struct __device_t *, void *);
	uint8_t *priv_data;
} filesystem_t;

typedef struct __mount_info_t {
	char *loc;
	struct __device_t *dev;
} mount_info_t;

typedef int fd_t;

#define VFS_FD_STDIN    0
#define VFS_FD_STDOUT   1
#define VFS_FD_STDERR   2
#define VFS_FD_DEBUG    3

int VFS_Write(fd_t file, uint8_t* data, size_t size);


uint8_t VFS_read(char *f, char* buf);
uint32_t VFS_ls(char *d, char *buf);
uint8_t VFS_exist_in_dir(char *wd, char* fn);
void VFS_init();
uint8_t list_mount();
uint8_t device_try_to_mount(struct __device_t *dev, char *location);