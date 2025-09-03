CC = aarch64-linux-gnu-gcc
LD = aarch64-linux-gnu-ld
OBJCOPY = aarch64-linux-gnu-objcopy

CFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib
ASFLAGS = 

# Find all .c and .S files in src/
C_SOURCES = $(wildcard src/*.c)
ASM_SOURCES = $(wildcard src/*.S)

# Convert source files to object files in build/
C_OBJECTS = $(C_SOURCES:src/%.c=build/%.o)
ASM_OBJECTS = $(ASM_SOURCES:src/%.S=build/%.o)
OBJECTS = $(C_OBJECTS) $(ASM_OBJECTS)

all: clean build build/kernel8.img

clean:
	rm -rf build

build:
	mkdir -p build

# Pattern rule for compiling .c files
build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

# Pattern rule for assembling .S files
build/%.o: src/%.S | build
	$(CC) $(ASFLAGS) -c $< -o $@

build/kernel8.img: $(OBJECTS)
	$(LD) -T src/linker.ld -o build/kernel8.elf $(OBJECTS)
	$(OBJCOPY) -O binary build/kernel8.elf build/kernel8.img

run: all
	qemu-system-aarch64 -M raspi3b -kernel build/kernel8.img -display none -d in_asm

qemu_debug: all
	qemu-system-aarch64 -M raspi3b -kernel build/kernel8.img -display none -S -s

.PHONY: all clean run