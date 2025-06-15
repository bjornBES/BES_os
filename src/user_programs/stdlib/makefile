TARGET_ASMFLAGS += -f elf
TARGET_CFLAGS += -ffreestanding -nostdlib -I. -I $(INCLUDE_DIR)

ProgramName = stdlib
ProgBUILD_DIR = $(BUILD_DIR)/$(ProgramName)
BUILD_DIRASM = $(ProgBUILD_DIR)/asm
BUILD_DIRC = $(ProgBUILD_DIR)/c

HEADERS_ASM = $(shell find . -type f -name "*.inc")

SOURCES_ASM = $(shell find . -type f -name "*.asm")

OBJECTS_ASM = $(patsubst %.asm, $(BUILD_DIRASM)/%.obj, $(SOURCES_ASM))

HEADERS_C = $(shell find . -type f -name "*.h")

SOURCES_C = $(shell find . -type f -name "*.c")

OBJECTS_C = $(patsubst %.c, $(BUILD_DIRC)/%.obj, $(SOURCES_C))

OutputFile = $(abspath $(BUILD_DIR)/$(ProgramName).a)


.PHONY: all clean always

all: $(OutputFile)

$(OutputFile): $(OBJECTS_ASM) $(OBJECTS_C)
	@mkdir -p $(dir $@)
	@$(TARGET_AR) rcs $@ $^
	@echo "--> Archived:  $(ProgramName).a"

$(BUILD_DIRC)/%.obj: %.c $(HEADERS_C)
	@mkdir -p $(@D)
	@$(TARGET_CC) $(TARGET_CFLAGS) -c -o $@ $<
	@echo "--> Compiled: " $<

$(BUILD_DIRASM)/%.obj: %.asm $(HEADERS_ASM)
	@mkdir -p $(@D)
	@$(TARGET_ASM) $(TARGET_ASMFLAGS) -o $@ $<
	@echo "--> Compiled: " $<

clean:
	@rm -rf $(ProgBUILD_DIR)
	@rm -f $(OutputFile)

always:
	mkdir -p $(ProgBUILD_DIR)
	mkdir -p $(BUILD_DIRASM)
