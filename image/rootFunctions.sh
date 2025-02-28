# /bin/bash

if [ -z "$1" ]; then
	echo "Please supply the correct arguments!"
	echo "Usage: ./rootFunction.sh <function>"
	exit 1
fi

if [ "$1" == "enter" ]; then
    sudo -p "Password to sudo" -s
elif [ "$1" == "exit" ]; then
    logout
fi
exit 0