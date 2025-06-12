BUILD_DIRUSER = $(BUILD_DIR)/user

SUBDIRS := $(shell find . -mindepth 1 -maxdepth 1 -type d)


.PHONY: all $(SUBDIRS) clean

all: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@ BUILD_DIR=$(abspath $(BUILD_DIR)/user)

clean: $(SUBDIRS)
	$(MAKE) -C $@ BUILD_DIR=$(abspath $(BUILD_DIR)/user) clean

always:
	mkdir -p $(BUILD_DIR)/user