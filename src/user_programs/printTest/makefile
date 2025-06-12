TARGET_LINKFLAGS += -T ../linker.ld -nostdlib
TARGET_ASMFLAGS += -f elf

HEADERS_ASM = $(shell find . -type f -name "*.inc")

SOURCES_ASM = $(shell find . -type f -name "*.asm")

OBJECTS_ASM = $(patsubst %.asm, $(BUILD_DIRASM)/%.obj, $(SOURCES_ASM))

ProgBUILD_DIR = $(BUILD_DIR)/$(ProgramName)
BUILD_DIRASM = $(ProgBUILD_DIR)/asm
OutputFile = $(abspath $(BUILD_DIR)/$(ProgramName).bin)

ProgramName = print

.PHONY: all clean always

all: $(OutputFile)

$(OutputFile): $(BUILD_DIRASM)/print.obj
	@$(TARGET_LD) $(TARGET_LINKFLAGS) -D DEBUG=1 -Wl,-Map=$(ProgBUILD_DIR)/$(ProgramName).map -o $@ $^
	@echo "--> Created:  " $(ProgramName) ".elf"

$(BUILD_DIRASM)/print.obj: program.asm
	@mkdir -p $(@D)
	@$(TARGET_ASM) $(TARGET_ASMFLAGS) -o $@ $<
	@echo "--> Compiled: " $<

clean:
	echo $(ProgBUILD_DIR)

always:
	mkdir -p $(ProgBUILD_DIR)
	mkdir -p $(BUILD_DIRASM)