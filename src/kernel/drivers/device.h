#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct __fs_t {
	char *name;
	uint8_t (*probe)();
	uint8_t (*read)(char *, char *, void *);
	uint8_t (*read_dir)(char *, char *, void *);
	uint8_t (*touch)(char *fn, void *);
	uint8_t (*writefile)(char *fn, char *buf, uint32_t len, void *);
	uint8_t (*exist)(char *filename,  void *);
	uint8_t (*mount)(void *);
	uint8_t *priv_data;
} filesystemInfo_t;
typedef struct device_t
{
    uint32_t id;
    filesystemInfo_t *fs;
    uint8_t (*read)(uint8_t* buffer, uint32_t offset , uint32_t len);
	uint8_t (*write)(uint8_t *buffer, uint32_t offset, uint32_t len);
} device_t;

void initDevice();
int addDevice(device_t* dev);
