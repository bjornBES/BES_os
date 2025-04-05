# /bin/bash

if [ -z "$1" ] || [ -z "$2" ] || [ -z "$3" ]; then
	echo "Please supply the correct arguments!"
	echo "Usage: ./mountDisk.sh <target> <filesystemType> <reserved_sectors>"
	exit 1
fi

target=$1
filesystemType=$2
reserved_sectors=$3

if [ "$filesystemType" == "fat12" ] || [ "$filesystemType" == "fat16" ] || [ "$filesystemType" == "fat32" ]; then
    fat_type="${filesystemType:3}"
    echo "mkfs.fat -F "$fat_type" -n BESOS -R "$reserved_sectors" "$target""
    mkfs.fat -F "$fat_type" -n BESOS -R "$reserved_sectors" -s 2 "$target"
elif ["$filesystemType" == "ext2"]; then
    mkfs.ext2 -L BESOS $target
else
    echo "Unsupported filesystem $filesystemType"
    exit 1
fi
