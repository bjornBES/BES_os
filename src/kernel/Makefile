TARGET_ASMFLAGS += -f elf
TARGET_CFLAGS += -ffreestanding -nostdlib -I. -I $(SOURCE_DIR)/src/libs -I $(INCLUDE_DIR)
TARGET_LIBS += -lgcc
TARGET_LINKFLAGS += -T linker.ld -nostdlib
CBLFLAGS += 

BUILD_DIRKERNEL = $(BUILD_DIR)/kernel
BUILD_DIRC = $(BUILD_DIRKERNEL)/c
BUILD_DIRASM = $(BUILD_DIRKERNEL)/asm
BUILD_DIRCBL = $(BUILD_DIRKERNEL)/cobol
TEMP_DIRCBL = $(BUILD_DIRKERNEL)/cobol/temp

HEADERS_C = $(shell find . -type f -name "*.h")

SOURCES_C = $(shell find . -type f -name "*.c")
# $(BUILD_DIR)/libcore.o

OBJECTS_C = $(patsubst %.c, $(BUILD_DIRC)/%.obj, $(SOURCES_C))

HEADERS_ASM = $(shell find . -type f -name "*.inc")

SOURCES_ASM = $(shell find . -type f -name "*.asm")

OBJECTS_ASM = $(patsubst %.asm, $(BUILD_DIRASM)/%.obj, $(SOURCES_ASM))

# SOURCES_CBL = $(shell find . -type f -name "*.cbl")

SOURCES_CBL := $(shell find . -type f -name "*.cbl" | sed 's|^\./||')

TEMP_CBL_C := $(patsubst %.cbl, $(TEMP_DIRCBL)/%, $(SOURCES_CBL))
OBJECTS_CBL := $(patsubst %.cbl, $(BUILD_DIRCBL)/%.obj, $(SOURCES_CBL))

.PHONY: all kernel clean always

all: kernel

kernel: $(BUILD_DIRKERNEL)/kernel.elf

$(BUILD_DIRKERNEL)/kernel.elf: $(OBJECTS_ASM) $(OBJECTS_C) $(OBJECTS_CBL)
	@bash ./link.sh $(TARGET_LD) $(TARGET_LINKFLAGS) -D DEBUG=1 -D __i370__=1 -D=PAGING=$(PAGING_ENABLE) -Wl,-Map=$(BUILD_DIRKERNEL)/kernel.map -o $@ $^ $(TARGET_LIBS)
#	@$(TARGET_LD) $(TARGET_LINKFLAGS) -D DEBUG=1 -D=PAGING=$(PAGING_ENABLE) -Wl,-Map=$(BUILD_DIRKERNEL)/kernel.map -o $@ $^ $(TARGET_LIBS)
	@echo "--> Created:  kernel.elf"

$(BUILD_DIRC)/%.obj: %.c $(HEADERS_C)
	@mkdir -p $(@D)
	@$(TARGET_CC) $(TARGET_CFLAGS) -D__minPages__=$(MIN_NUMBER_PAGES) -c -o $@ $<
	@echo "--> Compiled: " $<

$(BUILD_DIRASM)/%.obj: %.asm $(HEADERS_ASM)
	@mkdir -p $(@D)
	@$(TARGET_ASM) $(TARGET_ASMFLAGS) -DPAGING=$(PAGING_ENABLE) -o $@ $<
	@echo "--> Compiled: " $<

$(TEMP_DIRCBL)/%.c: %.cbl
	@mkdir -p $(@D)
	@$(CBL) $(CBLFLAGS) -C -o $@ $<
	@echo "--> Translated COBOL to C: " $< " into " $@

$(BUILD_DIRCBL)/%.obj: $(TEMP_DIRCBL)/%.c
	@mkdir -p $(@D)
	@$(TARGET_CC) $(TARGET_CFLAGS) -c -o $@ $<
	@echo "--> Compiled COBOL-C: " $<

always:
	@if not exist $(BUILD_DIRASM) mkdir $(BUILD_DIRASM)
	@if not exist $(BUILD_DIRC) mkdir $(BUILD_DIRC)
	@if not exist $(BUILD_DIRCBL) mkdir $(BUILD_DIRCBL)
	@if not exist $(TEMP_DIRCBL) mkdir $(TEMP_DIRCBL)

# arch/i686/isrs_gen.c arch/i686/isrs_gen.inc:
# @$(SOURCE_DIR)/build_tools/generate_isrs.sh $@

clean:
	rm -f $(BUILD_DIRKERNEL)/kernel.elf