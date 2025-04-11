# /bin/bash

if [ -z "$1" ] || [ -z "$2" ] || [ -z "$3" ]; then
	echo "Please supply the correct arguments!"
	echo "Usage: ./mountDisk.sh <image> <mount dir> <mount_method>"
	exit 1
fi

image=$1
mount_dir=$2
mount_method=$3

if [ "$mount_method" == "guestfs" ]; then
	echo "guestmount -a $image -i --rw $mount_dir"
	guestmount -a $image -i --rw $mount_dir
else
	# we are in sudo bc of python
	echo "sudo mount -t vfat -o loop=/dev/loop0,fat=32 $image --target $mount_dir"
	sudo mount -t vfat -o loop,fat=32 $image --target $mount_dir
fi
