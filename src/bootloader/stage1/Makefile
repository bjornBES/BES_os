TARGET_LINKFLAGS += -T linker.ld -nostdlib
TARGET_ASMFLAGS += -f elf

.PHONY: all clean

all: stage1

stage1: $(BUILD_DIR)/stage1.bin

#$(BUILD_DIR)/stage1.bin: boot.asm
#	@$(ASM) $< -f bin -o $@
#	@echo "--> Created  stage1.bin"

$(BUILD_DIR)/stage1.bin: $(BUILD_DIR)/stage1/stage1.obj
	@$(TARGET_LD) $(TARGET_LINKFLAGS) -Wl,-Map=$(BUILD_DIR)/stage1.map -o $@ $<

$(BUILD_DIR)/stage1/stage1.obj: boot.asm
	mkdir -p $(@D)
	@$(TARGET_ASM) $(TARGET_ASMFLAGS) $< -DFILESYSTEM=$(imageFS) -o $@
	@echo "--> Created  stage1.bin"

clean:
	@rm -f $(BUILD_DIR)/stage1.bin