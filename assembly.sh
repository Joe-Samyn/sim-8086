#!/bin/bash

echo "Assembling $1..."
file_name=$1
bin_name="${file_name%.*}.out"
nasm ${file_name} -o ${bin_name}