

OSNAME=tranbyos


# -Werror (treat warnings as errors)
CFLAGS_BASE= -O0 -g -m32 -Wall -Wno-unused-function -fstrength-reduce -fomit-frame-pointer -finline-functions -nostdlib -nostartfiles -nodefaultlibs -ffreestanding

# CFLAGS=-std=c11 $(CFLAGS_BASE) -I./src/include # for i386 gcc tools
CFLAGS=-std=c11 $(CFLAGS_BASE) -m32 -I./src/include
CXXFLAGS=-std=c++14 $(CFLAGS_BASE) -fno-exceptions -fno-rtti -m32 -I./src/include
CXX64FLAGS=-std=c++14 $(CFLAGS_BASE) -fno-exceptions -fno-rtti -m64 -I./src/include

# CC=i386-elf-gcc
# CXX=i386-elf-g++ # doesn't exist w/brew cross tools
# LD=i386-elf-ld

CC=x86_64-elf-gcc
CXX=x86_64-elf-g++
LD=x86_64-elf-ld

BUILD_DIR=tools
SRC=src
OBJ_DIR=obj
BIN_DIR=bin
ASM_DIR=$(SRC)/asm
INC_DIR=$(SRC)/include

# these are not used, as some recommend not using wildcards
# in case you add or copy a source file.
# Note(steve): Could reconsider
#SFILES := $(wildcard $(SRC)/*.c)
#OFILES := $(wildcard $(OBJ_DIR)/*.o)

# QEMU version 0.14
QIMG="qemu-img"
QEMU="qemu-system-i386"
# Q (OSX App works best for Lion as of May2012)
# http://www.kju-app.org/
#QIMG=/Applications/Q.app/Contents/MacOS/qemu-img
#QEMU=/Applications/Q.app/Contents/MacOS/i386-softmmu.app/Contents/MacOS/i386-softmmu


all: build
	@echo "All..."

build: link
	@echo "Building..."

link: compile assemble
	@echo "Linking..."

	# i386 toolchain
	# $(LD) -T $(BUILD_DIR)/link.ld -o $(BIN_DIR)/$(OSNAME).bin $(OBJ_DIR)/start.o

	# x86_64 toolchain ( -M to print memory map)
	$(LD) -m elf_i386 -T $(BUILD_DIR)/link.ld -o $(BIN_DIR)/$(OSNAME).bin \
							$(OBJ_DIR)/start.o \
							$(OBJ_DIR)/kernel.o  \
							$(OBJ_DIR)/main.o  \
							$(OBJ_DIR)/scrn.o  \
							$(OBJ_DIR)/gdt.o   \
							$(OBJ_DIR)/isrs.o  \
							$(OBJ_DIR)/timer.o \
							$(OBJ_DIR)/task.o \