include build_tools/config.mk

.PHONY: all floppy_image bootloader kernel clean always tools_fat debug

all: floppy_image tools_fat

include build_tools/toolchain.mk

#
# Floppy image
#
floppy_image: $(BUILD_DIR)/image.img

$(BUILD_DIR)/image.img: bootloader kernel
	@bash ./build_tools/make_disk.sh $(imageType) $(imageFS) $(imageSize) $(arch) $(config)

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
	@$(MAKE) -C src/bootloader/stage2 -j 4 BUILD_DIR=$(abspath $(BUILD_DIR))

#
# Kernel
#
kernel: $(BUILD_DIR)/kernel/kernel.elf # $(BUILD_DIR)/libcore.o

#$(BUILD_DIR)/libcore.o:
#	@$(MAKE) -C src/libs/core BUILD_DIR=$(abspath $(BUILD_DIR))

$(BUILD_DIR)/kernel/kernel.elf: always
	@$(MAKE) -C src/kernel -j 4 BUILD_DIR=$(abspath $(BUILD_DIR))

#
# Tools
#
tools_fat: $(BUILD_DIR)/tools/fat
$(BUILD_DIR)/tools/fat: always tools/fat/fat.c
	@mkdir -p $(BUILD_DIR)/tools
	@$(MAKE) -C tools/fat BUILD_DIR=$(abspath $(BUILD_DIR))

#
# user
#
user:
	@$(MAKE) -C src/user_programs BUILD_DIR=$(abspath $(BUILD_DIR))

runnow:
	bash run.sh disk $(BUILD_DIR)/image.img $(BUILD_DIR)/floppyImage.img $(BUILD_DIR)/sataImage.img
run: $(BUILD_DIR)/image.img
	bash run.sh disk $(BUILD_DIR)/image.img $(BUILD_DIR)/floppyImage.img $(BUILD_DIR)/sataImage.img
debug_flags:
	@echo "add -g"
	$(eval TARGET_ASM += -g)
	$(eval TARGET_CFLAGS += -g)

debug: debug_flags clean all

	@echo "running debug"
	bash debug.sh disk $(BUILD_DIR)/image.img $(BUILD_DIR)/floppyImage.img $(BUILD_DIR)/sataImage.img

debugnow:
	bash debug.sh disk $(BUILD_DIR)/image.img $(BUILD_DIR)/floppyImage.img $(BUILD_DIR)/sataImage.img


load: $(BUILD_DIR)/image.img
	@bash ./scripts/image/LoadImage.sh $(BUILD_DIR)/image.img /dev/sdd

#
# Always
#
always:
#	@echo "mkdir -p $(BUILD_DIR)" 
#	@mkdir -p $(BUILD_DIR)

toolchain:

#
# Clean
#
clean:
	@$(MAKE) -C src/bootloader/stage1 BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	@$(MAKE) -C src/bootloader/stage2 BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	@$(MAKE) -C src/kernel BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	@rm -rf $(BUILD_DIR)/*