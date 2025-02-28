TARGET_ASMFLAGS += -f elf -I.
TARGET_CFLAGS += -ffreestanding -nostdlib -I. -I $(SOURCE_DIR)/src/libs/core -I $(INCLUDE_DIR)
TARGET_CXXFLAGS += -ffreestanding -nostdlib -I. -I $(SOURCE_DIR)/src/libs/core -I $(INCLUDE_DIR)
TARGET_LIBS += -lgcc
TARGET_LINKFLAGS += -nostdlib


HEADERS_C = $(shell find . -type f -name "*.h")

SOURCES_C = $(shell find . -type f -name "*.c") \

OBJECTS_C = $(patsubst %.c, $(BUILD_DIR)/libcore/c/%.obj, $(SOURCES_C))

HEADERS_CXX = $(shell find . -type f -name "*.hpp")

SOURCES_CXX = $(shell find . -type f -name "*.cpp") \

OBJECTS_CXX = $(patsubst %.cpp, $(BUILD_DIR)/libcore/cpp/%.obj, $(SOURCES_CXX))

HEADERS_ASM = $(shell find . -type f -name "*.inc") \

SOURCES_ASM = $(shell find . -type f -name "*.asm")

OBJECTS_ASM = $(patsubst %.asm, $(BUILD_DIR)/libcore/asm/%.obj, $(SOURCES_ASM))

libcore: $(BUILD_DIR)/libcore.o

$(BUILD_DIR)/libcore.o: $(OBJECTS_ASM) $(OBJECTS_C) $(OBJECTS_CXX)
	@$(TARGET_LD) $(TARGET_LINKFLAGS) -shared -o $@ $^ $(TARGET_LIBS)
	@echo "--> Created:  kernel.bin"

$(BUILD_DIR)/libcore/cpp/%.obj: %.cpp $(HEADERS_CXX)
	@mkdir -p $(@D)
	@$(TARGET_CXX) $(TARGET_CXXFLAGS) -c -o $@ $<
	@echo "--> Compiled: " $<

$(BUILD_DIR)/libcore/c/%.obj: %.c $(HEADERS_C)
	@mkdir -p $(@D)
	@$(TARGET_CC) $(TARGET_CFLAGS) -c -o $@ $<
	@echo "--> Compiled: " $<

$(BUILD_DIR)/libcore/asm/%.obj: %.asm $(HEADERS_ASM)
	@mkdir -p $(@D)
	@$(TARGET_ASM) $(TARGET_ASMFLAGS) -o $@ $<
	@echo "--> Compiled: " $<