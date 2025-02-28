TARGET_ASMFLAGS += -f elf
TARGET_CFLAGS += -ffreestanding -nostdlib -I /usr/local/i686-elf/include -I ./ -I ./../../libs -I $(INCLUDE_DIR) -I $(TOOLCHAIN_DIR)/gcc-build-11.2.0/gcc/include
TARGET_LIBS += -lgcc
TARGET_LINKFLAGS += -T linker.ld -nostdlib

BUILD_DIRSTAGE2 = $(BUILD_DIR)/stage2
BUILD_DIRC = $(BUILD_DIRSTAGE2)/c
BUILD_DIRASM = $(BUILD_DIRSTAGE2)/asm


SOURCES_C=$(wildcard *.c)
SOURCES_ASM=$(wildcard *.asm)
OBJECTS_C=$(patsubst %.c, $(BUILD_DIRC)/%.obj, $(SOURCES_C))
OBJECTS_ASM=$(patsubst %.asm, $(BUILD_DIRASM)/%.obj, $(SOURCES_ASM))

.PHONY: all stage2 clean always

all: stage2

stage2: $(BUILD_DIRSTAGE2)/stage2.bin

$(BUILD_DIRSTAGE2)/stage2.bin: $(OBJECTS_ASM) $(OBJECTS_C)
	@$(TARGET_LD) $(TARGET_LINKFLAGS) -Wl,-Map=$(BUILD_DIRSTAGE2)/stage2.map -o $@ $^ $(TARGET_LIBS)
	@echo "--> Created  stage2.bin"

$(BUILD_DIRC)/%.obj: %.c
	mkdir -p $(@D)
	@$(TARGET_CC) $(TARGET_CFLAGS) -c -o $@ $<
	@echo "--> Compiled: " $<

$(BUILD_DIRASM)/%.obj: %.asm
	mkdir -p $(@D)
	@$(TARGET_ASM) $(TARGET_ASMFLAGS) -o $@ $<
	@echo "--> Compiled: " $<

always:
	@if not exist $(BUILD_DIRASM) mkdir $(BUILD_DIRASM)
	@if not exist $(BUILD_DIRC) mkdir $(BUILD_DIRC)


clean:
	@rm -f $(BUILD_DIRSTAGE2)/stage2.bin