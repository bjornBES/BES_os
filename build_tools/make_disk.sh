#/bin/bash

# $(TARGET_DIR) $(imageType) $(imageFS) $(imageSize) $(arch) $(config)
if [ -z "$1" ] || [ -z "$2" ] || [ -z "$3" ] || [ -z "$4" ] || [ -z "$5" ]; then
	echo "Please supply the correct arguments!"
	echo "Usage: ./make_disk.sh <image type> <target filesystem> <image size> <arch> <config>"
	exit 1
fi

imageType=$1
targetFilesystem=$2
imageSize=$3
arch=$4
config=$5

pyenv shell 3.14-dev

# python -m cProfile -o outputFile.txt -s time ./image/MakeImage.py $targetDir $imageType $targetFilesystem $imageSize $arch $config
python ./image/MakeImage.py $targetDir $imageType $targetFilesystem $imageSize $arch $config

echo "Done with all"
