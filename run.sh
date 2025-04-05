#!/bin/bash

QEMU_ARGS="-k da -debugcon stdio -vga std -m 8g -d guest_errors -netdev user,id=mynet0 -net nic,model=rtl8139,netdev=mynet0"

if [ "$#" -le 2 ]; then
    echo "Usage: ./run.sh <image_type> <image> <floppy_image> ..."
    exit 1
fi

IMAGE=$2
FLOPPY_IMAGE=$3
D3_IMAGE=$4

if [ "$1" == "floppy" ]; then
    QEMU_ARGS="${QEMU_ARGS} -fda $IMAGE"
    QEMU_ARGS="${QEMU_ARGS} -fdb $FLOPPY_IMAGE"
    QEMU_ARGS="${QEMU_ARGS} -drive format=raw,id=disk,if=none"
elif [ "$1" == "disk" ]; then
    QEMU_ARGS="${QEMU_ARGS} -fda $FLOPPY_IMAGE"
#    QEMU_ARGS="${QEMU_ARGS} -device floppy,id=fdc1"
    QEMU_ARGS="${QEMU_ARGS} -drive file=$IMAGE,format=raw,id=disk,if=ide"
    QEMU_ARGS="${QEMU_ARGS} -drive file=$D3_IMAGE,format=raw,id=sata_disk,if=none -device ahci,id=ahci -device ide-hd,drive=sata_disk"
#    QEMU_ARGS="${QEMU_ARGS} -device ide-hd,drive=disk"

else
    echo "Unknown image type: $1"
    exit 2
fi

QEMU_ARGS="${QEMU_ARGS} -boot order=c,menu=on"

QEMU_ARGS="${QEMU_ARGS} -device intel-hda"
QEMU_ARGS="${QEMU_ARGS} -device sb16"

qemu-system-x86_64 $QEMU_ARGS

#qemu:
#qemu-system-x86_64 -drive file=$(TARGET_IMG),format=raw,id=disk,if=none -device ahci,id=ahci -device ide-hd,drive=disk,bus=ahci.0 -m 1g -netdev user,id=mynet0 -net nic,model=rtl8139,netdev=mynet0

#qemu_dbg:
#qemu-system-x86_64 -drive file=$(TARGET_IMG),format=raw,id=disk,if=none -device ahci,id=ahci -device ide-hd,drive=disk,bus=ahci.0 -m 8g -netdev user,id=mynet0,hostfwd=udp::5555-:69,hostfwd=tcp::5555-:69 -net nic,model=rtl8139,netdev=mynet0 -object filter-dump,id=id,netdev=mynet0,file=../../netdmp.pcapng -s -smp 4 -no-shutdown -no-reboot

#qemu_iso:
#qemu-system-x86_64 -drive file=$(TARGET_ISO),format=raw -m 1g -netdev user,id=mynet0 -net nic,model=rtl8139,netdev=mynet0