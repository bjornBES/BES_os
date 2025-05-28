export CFLAGS = -Wall -Werror -I /usr/local/i686-elf/include -I ./ -I $(SOURCE_DIR)/src/libs -Wno-error=unused-variable -Wno-error=unused-function -Wno-error=unused-label -Wno-error=deprecated
# the -Wno-error=unused-variable flag is temp
export ASMFLAGS =
export CC = gcc
export CXX = g++
export LD = gcc
export ASM = nasm
export LINKFLAGS = -static
export LIBS = src/libs

export imageType = disk
export imageFS = fat32
export imageSize = 250m

export floppyOutput = $(BUILD_DIR)/image.img
export config = debug

export arch = i686
export TARGET = ${arch}-elf
binPath = $(TOOLCHAIN_DIR)/$(TARGET)/bin
export TARGET_ASM = nasm

export TARGET_ASMFLAGS = -f elf -I.
export TARGET_CFLAGS = $(CFLAGS) -std=c99 -nostdlib -ffreestanding #-O2
export TARGET_CXXFLAGS = $(CFLAGS) -std=c++17 -fno-exceptions -fno-rtti #-O2
export TARGET_LINKFLAGS = $(LINKFLAGS) -nostdlib

export TARGET_CC = ${binPath}/$(TARGET)-gcc
export TARGET_CXX = ${binPath}/$(TARGET)-g++
export TARGET_LD = ${binPath}/$(TARGET)-g++
export TARGET_LIBS =

export SOURCE_DIR = $(abspath .)
export BUILD_DIR = $(abspath build)/$(arch)_$(config)
export TOOLCHAIN_INCLUDE_DIR = $(TOOLCHAIN_DIR)/$(TARGET)/lib/gcc/$(TARGET)/$(GCC_VERSION)
export INCLUDE_DIR = $(TOOLCHAIN_INCLUDE_DIR)/include
export TOOLCHAIN_DIR = $(abspath toolchain)

BINUTILS_VERSION = 2.37
BINUTILS_URL = https://ftp.gnu.org/gnu/binutils/binutils-$(BINUTILS_VERSION).tar.xz

GCC_VERSION = 11.2.0
GCC_URL = https://ftp.gnu.org/gnu/gcc/gcc-$(GCC_VERSION)/gcc-$(GCC_VERSION).tar.xz

export PAGING_ENABLE = 0