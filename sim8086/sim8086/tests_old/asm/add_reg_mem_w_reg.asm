
; =====================================
; Add register or memory with register
; to either. 
; =====================================

bits 16

add bx, [bx + si]

add bx, cx
add bl, cl