# /bin/bash

if [ -z "$1" ] || [ -z "$2" ]; then
	echo "Please supply the correct arguments!"
	echo "Usage: ./LoadImage.sh <image> <drive>"
	exit 1
fi

image=$1
drive=$2

sudo umount $drive
sudo dd if=$image of=$drive status=progress && sync
