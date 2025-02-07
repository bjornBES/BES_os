include build_tools/config.mk

.PHONY: all floppy_image bootloader kernel clean always tools_fat

all: floppy_image tools_fat

include build_tools/toolchain.mk

MountDir = /mnt/disk

#
# Floppy image
#
floppy_image: $(BUILD_DIR)/main_floppy.img

$(BUILD_DIR)/main_floppy.img: bootloader kernel
	@dd if=/dev/zero of=$@ bs=512 count=2880
#	@mformat -i $(floppyOutput) -f 1440 ::
	@mkfs.fat -F 12 -n "BESOS" -v $(floppyOutput) 

	@mcopy -i $(floppyOutput) $(BUILD_DIR)/stage2.bin ::
	@mmd -i $(floppyOutput) boot
	@mcopy -i $(floppyOutput) $(BUILD_DIR)/kernel.bin ::boot/kernel.elf
	@mcopy -i $(floppyOutput) ./test.txt ::

	@mmd -i $(floppyOutput) mydir
	@mcopy -i $(floppyOutput) ./test.txt ::mydir/

	@dd if=$(BUILD_DIR)/stage1.bin of=$@ conv=notrunc
	@echo "--> Created: " $(floppyOutput)

#
# Bootloader
#
bootloader: stage1 stage2

stage1: $(BUILD_DIR)/stage1.bin

$(BUILD_DIR)/stage1.bin: always
	@$(MAKE) -C src/bootloader/stage1 BUILD_DIR=$(abspath $(BUILD_DIR))

stage2: $(BUILD_DIR)/stage2.bin

$(BUILD_DIR)/stage2.bin: always
	@$(MAKE) -C src/bootloader/stage2 BUILD_DIR=$(abspath $(BUILD_DIR))

#
# Kernel
#
kernel: $(BUILD_DIR)/kernel.bin

$(BUILD_DIR)/kernel.bin: always
	@$(MAKE) -C src/kernel BUILD_DIR=$(abspath $(BUILD_DIR))

#
# Tools
#
tools_fat: $(BUILD_DIR)/tools/fat
$(BUILD_DIR)/tools/fat: always tools/fat/fat.c
	@mkdir -p $(BUILD_DIR)/tools
	@$(MAKE) -C tools/fat BUILD_DIR=$(abspath $(BUILD_DIR))

#
# Always
#
always:
	@mkdir -p $(BUILD_DIR)

#
# Clean
#
clean:
	@$(MAKE) -C src/bootloader/stage1 BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	@$(MAKE) -C src/bootloader/stage2 BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	@$(MAKE) -C src/kernel BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	@rm -rf $(BUILD_DIR)/*