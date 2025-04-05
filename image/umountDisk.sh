# /bin/bash

if [ -z "$1" ] || [ -z "$2" ]; then
	echo "Please supply the correct arguments!"
	echo "Usage: ./umountDisk.sh <mount dir> <mount_method>"
	exit 1
fi

mount_dir=$1
mount_method=$2

if [ "$mount_method" == "guestfs" ]; then
	echo "guestumount $mount_dir"
	guestumount $mount_dir
else
	echo "umount $mount_dir"
	sudo umount $mount_dir
fi
