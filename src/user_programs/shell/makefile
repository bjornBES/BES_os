TARGET_ASMFLAGS += -f elf
TARGET_CFLAGS += -ffreestanding -nostdlib -I. -I ../stdlib/ -I $(INCLUDE_DIR)
TARGET_LINKFLAGS += -T ../linker.ld -nostdlib

LIBC := $(BUILD_DIR)/stdlib.a

ProgramName = shell
ProgBUILD_DIR = $(BUILD_DIR)/$(ProgramName)
BUILD_DIRASM = $(ProgBUILD_DIR)/asm
BUILD_DIRC = $(ProgBUILD_DIR)/c

HEADERS_ASM = $(shell find . -type f -name "*.inc")

SOURCES_ASM = $(shell find . -type f -name "*.asm")

OBJECTS_ASM = $(patsubst %.asm, $(BUILD_DIRASM)/%.obj, $(SOURCES_ASM))

HEADERS_C = $(shell find . -type f -name "*.h")

SOURCES_C = $(shell find . -type f -name "*.c")

OBJECTS_C = $(patsubst %.c, $(BUILD_DIRC)/%.obj, $(SOURCES_C))

OutputFile = $(abspath $(BUILD_DIR)/$(ProgramName).bin)


.PHONY: all clean always

all: $(OutputFile)

$(OutputFile): $(OBJECTS_ASM) $(OBJECTS_C)
	@$(TARGET_LD) $(TARGET_LINKFLAGS) -D DEBUG=1 -Wl,-Map=$(ProgBUILD_DIR)/$(ProgramName).map -o $@ $^ $(LIBC)
	@echo "--> Created:  $(ProgramName).elf"

$(BUILD_DIRC)/%.obj: %.c $(HEADERS_C)
	@mkdir -p $(@D)
	@$(TARGET_CC) $(TARGET_CFLAGS) -c -o $@ $<
	@echo "--> Compiled: " $<

$(BUILD_DIRASM)/%.obj: %.asm $(HEADERS_ASM)
	@mkdir -p $(@D)
	@$(TARGET_ASM) $(TARGET_ASMFLAGS) -o $@ $<
	@echo "--> Compiled: " $<

clean:
	echo $(ProgBUILD_DIR)

always:
	mkdir -p $(ProgBUILD_DIR)
	mkdir -p $(BUILD_DIRASM)