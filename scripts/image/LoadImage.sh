# /bin/bash

if [ -z "$1" ]; then
	echo "Please supply the correct arguments!"
	echo "Usage: ./LoadImage.sh <image>"
	exit 1
fi

image=$1

sudo dd if=$image of=/mnt/BESOS status=progress && sync
