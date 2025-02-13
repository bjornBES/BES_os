#/bin/bash

if [ -z "$1" ] || [ -z "$2" ]; then
	echo "Please supply the correct arguments!"
	echo "Usage: ./make_disk.sh <target dir> <target filesystem>"
	exit 1
fi

floppyOutput="./build/main_floppy.img"
BUILD_DIR="./build"

SIZE_IN_BYTES=$(du -sb "$1" | cut -f1)
SIZE_IN_BLOCKS=$((($SIZE_IN_BYTES / 512)))

if [ $SIZE_IN_BLOCKS -le 2880 ]; then

    SIZE_IN_BLOCKS=2880

fi


dd if=/dev/zero of=$floppyOutput bs=512 count=$SIZE_IN_BLOCKS
#mformat -i $floppyOutput -f 1440 ::

partition_offset=2048
reserved_sectors=1
Label="BESOS"
MkfsFlags="-R $reserved_sectors -n $Label --offset=$partition_offset -f 2 -s 1 -S 512 -v $floppyOutput"
if [ "$2" = "fat32" ]; then
    reserved_sectors+=1
    mkfs.vfat -F 32 $MkfsFlags
    echo "making fat32"
elif [ "$2" = "fat16" ]; then
    mkfs.fat -F 16 $MkfsFlags
    echo "making fat16"
elif [ "$2" = "fat12" ]; then
    mkfs.fat -F 12 $MkfsFlags
    echo "making fat12"
fi


# Ensure the target device exists
if [ ! -f "$floppyOutput" ]; then
    echo "Error: Target device $floppyOutput does not exist."
    exit 1
fi

loop_device=$(sudo losetup --show -Pf "$floppyOutput")
    
# Create a new msdos partition table
sudo parted -s "$loop_device" mklabel msdos

# Get free space in sectors
end_sector=$(sudo parted -s "$loop_device" unit s print free | awk '/Free Space/ {end=$2} END {print end}' | tr -d 's')

if [ -z "$end_sector" ]; then
    echo "Error: Unable to determine free space."
    sudo losetup -d "$loop_device"
    exit 1
fi

# Create a primary partition
sudo parted -s "$loop_device" mkpart primary ${partition_offset}s ${end_sector}s

# Set the boot flag
sudo parted -s "$loop_device" set 1 boot on

# Detach the loop device
sudo losetup -d "$loop_device"

echo "Partition table created successfully on $floppyOutput"


echo "Done with all"
