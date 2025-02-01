# Define the cross-compiler prefix
CROSS_COMPILE = riscv64-unknown-linux-gnu-
SYSROOT = /opt/riscv/sysroot
BUILD_DIR = $(CURDIR)/build
DEBUG_DIR = $(BUILD_DIR)/debug
RELEASE_DIR = $(BUILD_DIR)/release

# Define the compiler and other tools
CC = $(CROSS_COMPILE)gcc -march=rv32ima -mabi=ilp32 -static
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump

# Define the source files and the target executable
SRCS = main.c src/aes.c
OBJS = $(patsubst %.c,$(OUTPUT_DIR)/%.o,$(SRCS))
TARGET = aes_test

# External include directories
INCLUDES = -I$(SYSROOT)/usr/include -I../linux-on-litex-vexriscv/build/orange_crab_ctkey/software/include -Iinclude -I../../litex/litex/litex/soc/software/include -I../../litex/litex/litex/soc/cores/cpu/vexriscv 

# External library directories and libraries
LIBDIRS = -L$(SYSROOT)/usr/lib
LIBS = -l../linux-on-litex-vexriscv/build/orange_crab_ctkey/software/libc/libc.a

# Compiler flags
CFLAGS = -Wall $(INCLUDES)
RELEASE_CFLAGS = -Os
DEBUG_CFLAGS = -g -O0

# Linker flags
LDFLAGS = -L../linux-on-litex-vexriscv/build/orange_crab_ctkey/software

# Determine output directory based on build type
ifeq ($(DEBUG), 1)
	OUTPUT_DIR = $(DEBUG_DIR)
	CFLAGS += $(DEBUG_CFLAGS)
else
	CFLAGS += $(RELEASE_CFLAGS)
	OUTPUT_DIR = $(RELEASE_DIR)
endif

# Default target
all: $(OUTPUT_DIR)/$(TARGET)

# Debug target
debug:
	$(MAKE) DEBUG=1

# Link the target executable
$(OUTPUT_DIR)/$(TARGET): $(OBJS) | $(OUTPUT_DIR)
	$(CC) $(LDFLAGS) -o $@ $^

# Compile source files to object files
$(OUTPUT_DIR)/%.o: %.c | $(OUTPUT_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)/src

# Clean up build artifacts
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all debug clean