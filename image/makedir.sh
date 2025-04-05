# /bin/bash

if [ -z "$1" ]; then
	echo "Please supply the correct arguments!"
	echo "Usage: ./makedir.sh <dir> <UseSudo>"
	exit 1
fi

echo "in makedir.sh $2"

if [ $2 == "True" ]; then
	sudo mkdir -p $1
elif [ $2 == "False" ]; then
	mkdir -p $1
fi