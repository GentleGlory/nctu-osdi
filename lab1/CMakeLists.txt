cmake_minimum_required(VERSION 3.16)

# Set cross-compilation tools before project declaration
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_ASM_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_LINKER aarch64-linux-gnu-ld)
set(CMAKE_OBJCOPY aarch64-linux-gnu-objcopy)

# Set project name and languages
project(kernel8 C ASM)

# Set build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O2 -ffreestanding -nostdinc -nostdlib")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS}")

# Linker flags to disable standard startup files and libraries
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -nostdlib -nostartfiles")

# Enable languages after setting toolchain
enable_language(C ASM)

# Find all source files
file(GLOB C_SOURCES "kernel/*.c")
file(GLOB ASM_SOURCES "kernel/*.S")

# Create kernel8 executable
add_executable(kernel8.elf ${C_SOURCES} ${ASM_SOURCES})

# Add include directories
target_include_directories(kernel8.elf PRIVATE include)

# Set linker script
set_target_properties(kernel8.elf PROPERTIES
    LINK_FLAGS "-T ${CMAKE_CURRENT_SOURCE_DIR}/kernel/linker.ld"
)

# Custom command to create binary image
add_custom_command(
    TARGET kernel8.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary kernel8.elf kernel8.img
    COMMENT "Creating binary image kernel8.img"
    BYPRODUCTS kernel8.img
)

# Custom target for running in QEMU
add_custom_target(run
    COMMAND qemu-system-aarch64 -M raspi3b -kernel kernel8.img -display none -d in_asm
    DEPENDS kernel8.elf
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running kernel8.img in QEMU"
)

# Custom target for QEMU debug mode
add_custom_target(qemu_debug
    COMMAND qemu-system-aarch64 -M raspi3b -kernel kernel8.img -display none -S -s
    DEPENDS kernel8.elf
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running kernel8.img in QEMU debug mode"
)