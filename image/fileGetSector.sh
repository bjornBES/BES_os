# /bin/bash

if [ -z "$1" ] || [ -z "$2" ]; then
	echo "Please supply the correct arguments!"
	echo "Usage: ./mountDisk.sh <filePath> <image>"
	exit 1
fi

file=$1
image=$2

sudo mount -o loop $image_file /mnt

# Step 1: Get the size of the file
file_size=$(stat --format="%s" /mnt/$file)
# echo "File size: $file_size bytes" > /dev/tty

# Step 2: Get the sector size (assumed to be 512 bytes)
sector_size=512

# Step 3: Calculate the initial sector number (file offset / sector size)
initial_sector=$((file_size / sector_size))
# echo "Initial Sector Number: $initial_sector" > /dev/tty


# Step 4: Extract boot sector information to get the start of the data area
boot_sector=$(xxd -s 0 -l 512 $image | head -n 1)

# Extract necessary fields from boot sector (hex values)
reserved_sectors=$(echo $boot_sector | cut -d' ' -f15,16 | od -An -tu2)
fat_size=$(echo "$boot_sector" | cut -c37-40 | xxd -r -p | od -An -tu4)
first_data_sector=$((reserved_sectors + fat_size * 2))  # Typically 2 FAT tables

# Output results
echo "Reserved sectors:" > /dev/tty
echo "$reserved_sectors" > /dev/tty
echo "FAT size (sectors):" > /dev/tty
echo "$fat_size" > /dev/tty
echo "First data sector:" > /dev/tty
echo "$first_data_sector" > /dev/tty

# Step 5: Calculate the data sector for the file
data_sector=$((initial_sector + first_data_sector))
echo "Data Sector: $data_sector" > /dev/tty

# Step 6: Calculate the offset within the data sector
data_offset=$((file_size % sector_size))
echo "Offset within Data Sector: $data_offset" > /dev/tty

echo "$data_offset"
