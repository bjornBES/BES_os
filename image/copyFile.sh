# /bin/bash

if [ -z "$1" ] || [ -z "$2" ] || [ -z "$3" ]; then
	echo "Please supply the correct arguments!"
	echo "Usage: ./copyFile.sh <source> <destination> <source size>"
	exit 1
fi

source=$1
destination=$2
size=$3

sudo cp $source $destination
