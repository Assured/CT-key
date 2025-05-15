PLATFORM_BUILD_DIR = ./build/gsd_orangecrab
SOC_DIR = ../../litex/litex/litex/soc

include $(PLATFORM_BUILD_DIR)/software/include/generated/variables.mak
include $(SOC_DIR)/software/common.mak

BUILD_DIR = $(CURDIR)/build
DEBUG_DIR = $(BUILD_DIR)/debug
RELEASE_DIR = $(BUILD_DIR)/release

# Define the source files and the target executable
SRCS = main.c src/aes.c 
OBJECTS = $(patsubst %.c,$(OUTPUT_DIR)/%.o,$(SRCS))
OBJECTS += $(PLATFORM_BUILD_DIR)/software/bios/crt0.o
TARGET = firmware.bin
INCLUDES += -I./include

# Compiler flags
RELEASE_CFLAGS = -Os
DEBUG_CFLAGS = -g -O0

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

$(OUTPUT_DIR)/%.bin: $(OUTPUT_DIR)/%.elf
	$(OBJCOPY) -O binary $< $@
	chmod -x $@

vpath %.a $(PACKAGES:%=$(PLATFORM_BUILD_DIR)/software/%)

$(OUTPUT_DIR)/firmware.elf: $(OBJECTS)
	$(CC) $(LDFLAGS) -T linker.ld -N -o $@ \
		$(OBJECTS) \
		$(PACKAGES:%=-L$(PLATFORM_BUILD_DIR)/software/%) \
		-Wl,--whole-archive \
		-Wl,--gc-sections \
		-Wl,-Map,$@.map \
		$(LIBS:lib%=-l%)
	chmod -x $@

-include $(OBJECTS:.o=.d)

VPATH = $(BIOS_DIRECTORY):$(BIOS_DIRECTORY)/cmds:$(CPU_DIRECTORY)

main.o: main.c
	$(compile)

%.o: %.c
	$(compile)

%.o: %.S
	$(assemble)

# Debug target
debug:
	$(MAKE) DEBUG=1

# Compile source files to object files
$(OUTPUT_DIR)/%.o: %.c | $(OUTPUT_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)/src

# Clean up build artifacts
clean:
	rm -rf $(BUILD_DIR)/release
	rm -rf $(BUILD_DIR)/debug

.PHONY: all debug clean