

; =====================================
; Add an immediate value to a register
; or memory location. 
; =====================================

bits 16

add bl, 12
add cx, 1000

add [bp + di], 17
add [bp + di + 7], 26
add [bx - 1234], 45

add word [bp + di], 260
add word [bp + di], 8
add word [bp + di], -8

add word [bp + di + 7], -27