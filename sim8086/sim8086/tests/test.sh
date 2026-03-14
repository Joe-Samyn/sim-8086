#!/bin/bash
echo "Running 8086 simulator located at: $1"
echo "Using test file: $2"

# Run the Intel 8086 simulator with the provided path and test file 
$1 $2

# Assemble result
nasm result.asm -o result.bin

# Compare binary files 
if cmp -s result.bin mov_tst.bin; then
    echo "Files are equivalent."
    exit 0
else 
    echo "ERROR::Binary files are not equivalent."
    exit 1
fi
