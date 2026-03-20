

; =====================================
; Add an immediate value to a register
; or memory location. 
; =====================================

bits 16

add bl, 12
add [bp + di], 17
add [bp + di + 7], 26
add [BX - 1234], 45