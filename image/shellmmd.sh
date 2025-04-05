# /bin/bash

if [ -z "$1" ] || [ -z "$2" ]; then
	echo "Please supply the correct arguments!"
	echo "Usage: ./mountDisk.sh <image> <file_dst>"
	exit 1
fi

mmd -i $1 $2
