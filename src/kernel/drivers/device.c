#include "device.h"
#include "malloc.h"
#include "memory.h"

device_t* devices = 0;
uint8_t lastid = 0;

void initDevice()
{

}

int addDevice(device_t* dev)
{
    devices = (device_t*)malloc(sizeof(device_t)*64);
    memset(devices, 0, sizeof(device_t)*64);
    lastid = 0;
}