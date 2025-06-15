# /bin/bash

if [ -z "$1" ]; [ -z "$2" ]; then
	echo "Please supply the correct arguments!"
	echo "Usage: ./shelldd.sh <dir> <len>"
	exit 1
fi

disk=$1
len=$2
touch $disk
dd if=/dev/zero of=$disk status=progress count=$len bs=512
