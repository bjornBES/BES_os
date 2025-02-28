# /bin/bash

if [ -z "$1" ] || [ -z "$2" ]; then
	echo "Please supply the correct arguments!"
	echo "Usage: ./mountDisk.sh <image> <mount dir>"
	exit 1
fi

image=$1
mount_dir=$2

# we are in sudo bc of python

echo "sudo mount -t vfat -o loop=/dev/loop0,fat=32 $image --target $mount_dir"
sudo mount -t vfat -o loop,fat=32 $image --target $mount_dir

