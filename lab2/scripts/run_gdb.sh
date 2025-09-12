#!/bin/bash

# Auto GDB connection script for ARM64 kernel debugging
# Usage: ./run_gdb.sh [source_directory]

SOURCE_DIR=${1:-".."}

echo "Starting GDB for ARM64 kernel debugging..."
echo "Make sure QEMU is running with -s -S options"
echo "Using source directory: $SOURCE_DIR"

gdb-multiarch kernel8.elf \
    -ex "target remote :1234" \
    -ex "set architecture aarch64" \
    -ex "directory $SOURCE_DIR"