#!/bin/bash

scripts=(
    "./decodeTests_MOV/test_decode_mov.sh"
    "./decodeTests_ADD/test_decode_add.sh"
    "./decodeTests_CMP/test_decode_cmp.sh"
    "./decodeTests_JMP/test_decode_jmp.sh"
    "./decodeTests_DEC/test_decode_dec.sh"
    "./decodeTests_PUSHPOP/test_decode_pushpop.sh"
    "./decodeTests_SUB/test_decode_sub.sh"
    "./decodeTests_XCHG/test_decode_xchg.sh"
)

for script in "${scripts[@]}"; do 
    chmod +x "${script}"
    if ! [[ -x "${script}" ]]; then
        chmod +x "${script}"
    fi 

    "${script}"
done