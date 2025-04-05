BUILD_DIR?=build
CC?=gcc
CFLAGS?=-ggdb
CFLAGS := $(filter-out -Werror, $(CFLAGS))
CFLAGS := $(filter-out -Wall, $(CFLAGS))

SOURCES_C=$(wildcard *.c)
OBJECTS_C=$(patsubst %.c, $(BUILD_DIR)/tools/fat/%.o, $(SOURCES_C))

.PHONY: all fat clean always

all: fat

fat: $(BUILD_DIR)/tools/fat.out

$(BUILD_DIR)/tools/fat.out: $(OBJECTS_ASM) $(OBJECTS_C)
	@$(CC) $(CFLAGS) -o $@ $(OBJECTS_C)

$(BUILD_DIR)/tools/fat/%.o: %.c always
	@$(CC) $(CFLAGS) -c -o $@ $<

always:
# need to remove -Werror from CFLAGS
	@mkdir -p $(BUILD_DIR)/tools/fat

clean:
	@rm -f $(BUILD_DIR)/tools/fat.out
	@rm -rf $(BUILD_DIR)/tools/fat