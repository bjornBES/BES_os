import os
from decimal import Decimal
from io import SEEK_CUR, SEEK_SET
from pathlib import Path
from shutil import copy2
import parted
import sh

align_start = 2048
target = "./build/main_floppy.img"

device = parted.getDevice(target)
disk = parted.freshDisk(device, 'msdos')

freeSpace = disk.getFreeSpaceRegions()

partitionGeometry = parted.Geometry(device, align_start, end=freeSpace[-1].end)
partition = parted.Partition(disk=disk, type=parted.PARTITION_NORMAL, geometry=partitionGeometry)
partition.setFlag(parted.PARTITION_BOOT)

disk.addPartition(partition, constraint=device.optimalAlignedConstraint)
disk.commit()
