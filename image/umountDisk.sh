# /bin/bash

if [ -z "$1" ]; then
	echo "Please supply the correct arguments!"
	echo "Usage: ./umountDisk.sh <mount dir>"
	exit 1
fi

mount_dir=$1

echo "umount $mount_dir"
sudo umount $mount_dir
