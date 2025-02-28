import parted
import sys

def create_partition_table(target: str):
    device = parted.getDevice(target)
    disk = parted.freshDisk(device, 'msdos')
    freeSpace = disk.getFreeSpaceRegions()
    partitionGeometry = parted.Geometry(device, 512, end=freeSpace[-1].end)
    partition = parted.Partition(disk=disk, type=parted.PARTITION_NORMAL, geometry=partitionGeometry)
    partition.setFlag(parted.PARTITION_BOOT)
    disk.addPartition(partition, constraint=device.optimalAlignedConstraint)
    disk.commit()
    
if (sys.argv.__len__() < 2):
    print("Usage: partitons.py image")
    exit(1)
        
create_partition_table(sys.argv[1])
exit(0)