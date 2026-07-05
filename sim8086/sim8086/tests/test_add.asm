; ------------------------------------------------------
; Sample ADD instructions for the Intel 8086 processor
; ------------------------------------------------------
bits 16

add cx, bx
add [bx + si], cx
add [bx + si + 5], dx
add bx, 18
add word [bx + si + 17], 1024