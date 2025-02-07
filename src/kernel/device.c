/** @author Levente Kurusa <levex@linux.com> **/
#include <stdint.h>

#include "device.h"
#include "memory.h"
#include "malloc.h"
#include "stdio.h"

device_t *devices = 0;
uint8_t lastid = 0;

void device_init()
{
	devices = (device_t*)malloc(64*sizeof(device_t));
	memset(devices, 0, 64*sizeof(device_t));
	lastid = 0;
	printf("Device Manager initialized.\n");
}

void device_print_out()
{
	for(int i = 0; i < lastid; i++)
	{
		//if(!devices[lastid]) return;
		printf("id: %d, unique: %d, %s, %s\n", i, devices[i].unique_id,
				devices[i].dev_type == DEVICE_CHAR ?"CHAR":"BLOCK", devices[i].name);
	}
}

int device_add(device_t* dev)
{
	devices[lastid] = *dev;
	printf("Registered Device %s (%d) as Device#%d\n", dev->name, dev->unique_id, lastid);
	lastid++;
	return lastid-1;
}

device_t *device_get_by_id(uint32_t id)
{
	for(int i = 0;i < 64; i++)
	{
		if(devices[i].unique_id == id) return &devices[i];
	}
	return 0;
}

int device_getnumber()
{
	return lastid;
}

device_t *device_get(uint32_t id)
{
	return &devices[id];
}