TARGET_ASMFLAGS += -f elf
TARGET_CFLAGS += -ffreestanding -nostdlib -I. -I $(SOURCE_DIR)/src/libs -I $(INCLUDE_DIR) -I $(TOOLCHAIN_DIR)/gcc-build-11.2.0/gcc/include
TARGET_LIBS += -lgcc
TARGET_LINKFLAGS += -T linker.ld -nostdlib

BUILD_DIRC = $(BUILD_DIR)/kernel/c
BUILD_DIRASM = $(BUILD_DIR)/kernel/asm

HEADERS_C = $(wildcard *.h) \
			$(wildcard */*.h) \
			$(wildcard */*/*.h) \
			$(wildcard */*/*/*.h)

SOURCES_C = $(wildcard *.c) \
			$(wildcard */*.c) \
			$(wildcard */*/*.c) \
			$(wildcard */*/*/*.c) \
			arch/i686/isrs_gen.c

OBJECTS_C = $(patsubst %.c, $(BUILD_DIR)/kernel/c/%.obj, $(SOURCES_C))

HEADERS_ASM = $(wildcard *.inc) \
			  $(wildcard */*.inc) \
			  $(wildcard */*/*.inc) \
			  $(wildcard */*/*/*.inc) \
			  arch/i686/isrs_gen.inc

SOURCES_ASM = $(wildcard *.asm) \
			  $(wildcard */*.asm) \
			  $(wildcard */*/*.asm) \
			  $(wildcard */*/*/*.asm)


OBJECTS_ASM = $(patsubst %.asm, $(BUILD_DIR)/kernel/asm/%.obj, $(SOURCES_ASM))

.PHONY: all kernel clean always

all: kernel

kernel: $(TARGET_DIR)/bin/kernel.bin

$(TARGET_DIR)/bin/kernel.bin: $(OBJECTS_ASM) $(OBJECTS_C)
	@$(TARGET_LD) $(TARGET_LINKFLAGS) -D DEBUG=1 -Wl,-Map=$(BUILD_DIR)/kernel.map -o $@ $^ $(TARGET_LIBS)
	@echo "--> Created:  kernel.bin"

$(BUILD_DIR)/kernel/c/%.obj: %.c $(HEADERS_C)
	@mkdir -p $(@D)
	@$(TARGET_CC) $(TARGET_CFLAGS) -c -o $@ $<
	@echo "--> Compiled: " $<

$(BUILD_DIR)/kernel/asm/%.obj: %.asm $(HEADERS_ASM)
	@mkdir -p $(@D)
	@$(TARGET_ASM) $(TARGET_ASMFLAGS) -o $@ $<
	@echo "--> Compiled: " $<

always:
	@if not exist $(BUILD_DIRASM) mkdir $(BUILD_DIRASM)
	@if not exist $(BUILD_DIRC) mkdir $(BUILD_DIRC)

# arch/i686/isrs_gen.c arch/i686/isrs_gen.inc:
# @$(SOURCE_DIR)/build_tools/generate_isrs.sh $@

clean:
	rm -f $(TARGET_DIR)/bin/kernel.bin