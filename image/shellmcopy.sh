# /bin/bash

if [ -z "$1" ] || [ -z "$2" ] || [ -z "$3" ]; then
	echo "Please supply the correct arguments!"
	echo "Usage: ./mountDisk.sh <image> <file_src> <file_dst>"
	exit 1
fi

mcopy -i $1 $2 $3
