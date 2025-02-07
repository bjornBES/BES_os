#pragma once
#include <stdint.h>
#include "device.h"

void devfs_init();
uint8_t devfs_probe(device_t *dev);
uint8_t devfs_mount(device_t *dev, void *priv);
