
#pragma onces

#include <stdint.h>
#include "device.h"

void proc_init();
uint8_t procfs_probe(device_t *dev);
uint8_t procfs_mount(device_t *dev, void *priv);