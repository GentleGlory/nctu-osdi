#!/bin/bash

# Check if correct number of arguments provided
if [ $# -ne 2 ]; then
    echo "Usage: $0 <text_file> <tty_device>"
    echo "Example: $0 test.txt /dev/ttyUSB0"
    exit 1
fi

TEXT_FILE="$1"
TTY_DEVICE="$2"

# Check if text file exists
if [ ! -f "$TEXT_FILE" ]; then
    echo "Error: Text file '$TEXT_FILE' not found"
    exit 1
fi

# Check if TTY device exists
if [ ! -e "$TTY_DEVICE" ]; then
    echo "Error: TTY device '$TTY_DEVICE' not found"
    exit 1
fi

echo "Sending file '$TEXT_FILE' to '$TTY_DEVICE'..."

# Send file content to TTY
cat "$TEXT_FILE" > "$TTY_DEVICE"

# Send Ctrl+C (ASCII 3) to TTY
printf '\003' > "$TTY_DEVICE"

echo "File transmission completed."