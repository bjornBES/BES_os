#include "device.h"
#include "malloc.h"
#include "memory.h"
#include "stdio.h"
#include "debug.h"

#define MODULE "DEVICE"

device_t **devices = 0;
Page* devicesPage;
uint8_t lastid = 0;

void initDevice()
{
	devices = (device_t **)malloc(64 * sizeof(device_t), devicesPage);
	memset(devices, 0, 64 * sizeof(device_t));
	lastid = 0;
}

void PrintDeviceOut()
{
	log_debug(MODULE, "sizeof device %u", sizeof(device_t));
	for (int i = 0; i < lastid; i++)
	{
		// if(!devices[lastid]) return;
		log_info(MODULE, "id: %d, unique: %d, %s, %s", i, devices[i]->id, devices[i]->dev_type == DEVICE_CHAR ? "CHAR" : "BLOCK", devices[i]->name);
	}
}

int addDevice(device_t *dev)
{
	if (dev == NULL)
	{
		log_err(MODULE, "Cannot add a NULL device");
		return -1;
	}

	if (dev->id == 0xFFFFFFFF)
	{
		dev->id = lastid;
	}
	devices[lastid] = dev;
	lastid++;
	return lastid - 1;
}

device_t *GetDeviceUsingId(uint32_t id)
{
	for (int i = 0; i < 64; i++)
	{
		if (devices[i]->id == id)
		{
			return devices[i];
		}
		else
		{
			continue;
		}
	}
	return 0;
}

device_t *GetDevice(uint32_t id)
{
	return devices[id];
}