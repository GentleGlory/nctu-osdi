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
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -ffreestanding -nostdinc -nostdlib")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS}")

# Add debug info and optimization based on build type
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0")
    set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -g")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O2")
    set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -g")
endif()

# Linker flags to disable standard startup files and libraries
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -nostdlib -nostartfiles")

# Enable languages after setting toolchain
enable_language(C ASM)

# Find source files
file(GLOB COMMON_C_SOURCES "core/*.c" "drivers/*.c" "libs/*.c")

file(GLOB KERNEL_C_SOURCES "kernel/*.c")
file(GLOB KERNEL_ASM_SOURCES "kernel/*.S")

file(GLOB BOOTLOADER_C_SOURCES "bootloader/*.c")
file(GLOB BOOTLOADER_ASM_SOURCES "bootloader/*.S")


# bootloader is named kernel8.img
# Create kernel8 executable (bootloader includes common sources)
add_executable(kernel8.elf ${COMMON_C_SOURCES} ${BOOTLOADER_C_SOURCES} ${BOOTLOADER_ASM_SOURCES})

# Add include directories
target_include_directories(kernel8.elf PRIVATE include)

# Add BOOTLOADER define for bootloader target
target_compile_definitions(kernel8.elf PRIVATE BOOTLOADER)

# Set linker script
set_target_properties(kernel8.elf PROPERTIES
    LINK_FLAGS "-T ${CMAKE_CURRENT_SOURCE_DIR}/bootloader/linker.ld"
)

# Custom command to create binary image
add_custom_command(
    TARGET kernel8.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary kernel8.elf kernel8.img
    COMMENT "Creating binary image kernel8.img"
    BYPRODUCTS kernel8.img
)

# kernel is named kernel_real.img
# Create kernel executable (includes common sources and kernel sources)
add_executable(kernel_real.elf ${COMMON_C_SOURCES} ${KERNEL_C_SOURCES} ${KERNEL_ASM_SOURCES})

# Add include directories
target_include_directories(kernel_real.elf PRIVATE include)

# Set linker script
set_target_properties(kernel_real.elf PROPERTIES
    LINK_FLAGS "-T ${CMAKE_CURRENT_SOURCE_DIR}/kernel/linker.ld"
)

# Custom command to create binary image
add_custom_command(
    TARGET kernel_real.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary kernel_real.elf kernel_real.img
    COMMENT "Creating binary image kernel_real.img"
    BYPRODUCTS kernel_real.img
)

# Custom target for running bootloader in QEMU
add_custom_target(run
    COMMAND qemu-system-aarch64 -M raspi3b -kernel kernel8.img -serial stdio
    DEPENDS kernel8.elf
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running kernel8.img (bootloader) in QEMU"
)

# Custom target for running kernel directly in QEMU
add_custom_target(run_kernel
    COMMAND qemu-system-aarch64 -M raspi3b -kernel kernel_real.img -serial stdio
    DEPENDS kernel_real.elf
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running kernel_real.img in QEMU"
)

# Copy GDB script to build directory (always check for updates)
add_custom_target(copy_gdb_script ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/scripts/run_gdb.sh ${CMAKE_BINARY_DIR}/run_gdb.sh
    COMMENT "Copying GDB script to build directory"
    VERBATIM
)

# Copy send_file script to build directory (always check for updates)
add_custom_target(copy_send_file_script ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/scripts/send_file.sh ${CMAKE_BINARY_DIR}/send_file.sh
    COMMENT "Copying send_file script to build directory"
    VERBATIM
)

# Copy send_kernel.py to build directory (always check for updates)
add_custom_target(copy_send_kernel ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/scripts/send_kernel.py ${CMAKE_BINARY_DIR}/send_kernel.py
    COMMENT "Copying send_kernel.py script to build directory"
    VERBATIM
)

# Custom target for QEMU debug mode (bootloader)
add_custom_target(qemu_debug
    COMMAND qemu-system-aarch64 -M raspi3b -kernel kernel8.img -S -s -serial stdio
    DEPENDS kernel8.elf
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running kernel8.img (bootloader) in QEMU debug mode"
)

# Custom target for QEMU debug mode (kernel)
add_custom_target(qemu_debug_kernel
    COMMAND qemu-system-aarch64 -M raspi3b -kernel kernel_real.img -S -s -serial stdio
    DEPENDS kernel_real.elf
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running kernel_real.img in QEMU debug mode"
)