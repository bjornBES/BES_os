export CFLAGS = -std=c99 -g -I /usr/local/i686-elf/include -I ./ -I $(SOURCE_DIR)/src/libs 
export ASMFLAGS =
export CC = gcc
export CXX = g++
export LD = gcc
export ASM = nasm
export LINKFLAGS = -static
export LIBS =

export imageType = disk
export imageFS = fat32
export imageSize = 250m

export floppyOutput = $(BUILD_DIR)/main_floppy.img

export TARGET = i686-elf
binPath = $(TOOLCHAIN_DIR)/$(TARGET)/bin
export TARGET_ASM = nasm
export TARGET_ASMFLAGS =
export TARGET_CFLAGS = -std=c99 -g #-O2
export TARGET_CC = ${binPath}/$(TARGET)-gcc
export TARGET_CXX = ${binPath}/$(TARGET)-g++
export TARGET_LD = ${binPath}/$(TARGET)-gcc
export TARGET_LINKFLAGS =
export TARGET_LIBS =

export SOURCE_DIR = $(abspath .)
export BUILD_DIR = $(abspath build)
export INCLUDE_DIR = $(abspath include)
export TOOLCHAIN_DIR = $(abspath toolchain)
export TARGET_DIR = $(abspath target)

BINUTILS_VERSION = 2.37
BINUTILS_URL = https://ftp.gnu.org/gnu/binutils/binutils-$(BINUTILS_VERSION).tar.xz

GCC_VERSION = 11.2.0
GCC_URL = https://ftp.gnu.org/gnu/gcc/gcc-$(GCC_VERSION)/gcc-$(GCC_VERSION).tar.xz