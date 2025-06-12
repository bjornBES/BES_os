#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "malloc.h"

typedef enum __device_type {
	DEVICE_UNKNOWN = 0,
	DEVICE_CHAR = 1,
	DEVICE_BLOCK = 2,
} device_type;

struct device_t;

typedef struct __device_t
{
	char* name;
    uint32_t id;
    struct __fs_t  *fs;
	device_type dev_type;
    uint32_t (*read)(void* buffer, uint64_t offset , uint32_t len, struct __device_t* device);
	uint32_t (*write)(void *buffer, uint64_t offset, uint32_t len, struct __device_t* device);
	void* priv;
} device_t;

typedef struct __fs_t {
	char *name;
	bool (*probe)(device_t* dev);
	bool (*read)(char *, uint8_t *, device_t* dev, void *);
	bool (*read_dir)(char *, uint8_t *, device_t* dev, void *);
	bool (*find_entry)(char *, void*, device_t* dev, void *);
	bool (*touch)(char *fn, device_t* dev, void *);
	bool (*writefile)(char *fn, uint8_t *buf, uint32_t len, device_t* dev, void *);
	bool (*exist)(char *filename, device_t* dev,  void *);
	bool (*mount)(device_t* dev, void *);
	bool (*getRoot)(void*, device_t* dev, void *);
	bool *priv_data;
} filesystemInfo_t;

extern Page* devicePage;
extern Page* privPage;

void initDevice();
void PrintDeviceOut();
int addDevice(device_t* dev);
device_t *GetDeviceUsingId(uint32_t id);
device_t *GetDevice(uint32_t id);
