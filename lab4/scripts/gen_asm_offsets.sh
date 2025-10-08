#!/bin/sh
# Generate asm-offsets.h from asm-offsets.s

ASM_OFFSETS_S="$1"
ASM_OFFSETS_H="$2"

# Create output directory if it doesn't exist
mkdir -p "$(dirname "$ASM_OFFSETS_H")"

# Generate header file
cat > "$ASM_OFFSETS_H" << 'EOF'
/* Automatically generated file - do not edit */

#ifndef _ASM_OFFSETS_H
#define _ASM_OFFSETS_H

EOF

# Extract #define lines from assembly
grep -E '^#define' "$ASM_OFFSETS_S" >> "$ASM_OFFSETS_H"

# Add closing guard
cat >> "$ASM_OFFSETS_H" << 'EOF'

#endif /* _ASM_OFFSETS_H */
EOF
