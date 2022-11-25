MAKEFILE_DIR := $(dir $(abspath  $(lastword $(MAKEFILE_LIST))))

BUILD_CONF ?= Debug
#BUILD_CONF := Release

PROJ_NAME 	:= music_box
OUTPUT_DIR 	:= build/$(BUILD_CONF)

ELF_FILE_PATH := $(OUTPUT_DIR)/$(PROJ_NAME).elf


$(ELF_FILE_PATH): build

.PHONY: build
build:
	mkdir -p $(OUTPUT_DIR)
	cmake                                               \
        -DCMAKE_BUILD_TYPE=$(BUILD_CONF)                \
		-DPSG_MML_PROJECT_NAME=$(PROJ_NAME)             \
        -S $(MAKEFILE_DIR)                              \
        -B $(OUTPUT_DIR)
	make -C  $(OUTPUT_DIR)

.PHONY: clean
clean:
	rm -rf $(OUTPUT_DIR)

.PHONY: rebuild
rebuild: clean build
	
