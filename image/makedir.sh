# /bin/bash

if [ -z "$1" ]; then
	echo "Please supply the correct arguments!"
	echo "Usage: ./makedir.sh <dir>"
	exit 1
fi

mkdir -p $1
