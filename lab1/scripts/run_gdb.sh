#!/bin/bash

# Auto GDB connection script for ARM64 kernel debugging

echo "Starting GDB for ARM64 kernel debugging..."
echo "Make sure QEMU is running with -s -S options"

gdb-multiarch kernel8.elf \
    -ex "target remote :1234" \
    -ex "set architecture aarch64"