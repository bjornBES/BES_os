#!/bin/bash

QEMU_ARGS="-S -gdb stdio -m 8g -d guest_errors -netdev user,id=mynet0 -net nic,model=rtl8139,netdev=mynet0"

if [ "$#" -le 1 ]; then
    echo "Usage: ./debug.sh <image_type> <image> <floppy_image>"
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
#    QEMU_ARGS="${QEMU_ARGS} -drive file=$D3_IMAGE,format=raw,id=ide_disk,if=none -device ide-hd,drive=ide_disk"

else
    echo "Unknown image type: $1"
    exit 2
fi

QEMU_ARGS="${QEMU_ARGS} -device intel-hda"
QEMU_ARGS="${QEMU_ARGS} -device sb16"

# b *0x7c00
# layout asm
cat > .vscode/.gdb_script.gdb << EOF
    symbol-file $PWD/build/i686_debug/kernel/kernel.elf
    set disassembly-flavor intel
    target remote | qemu-system-i386 $QEMU_ARGS
EOF

gdb -x .vscode/.gdb_script.gdb