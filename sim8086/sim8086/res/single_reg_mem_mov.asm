

; =============================
; This file represents a basic 
; register to memory write in 
; Intel 8086 assembly.
; =============================


bits 16

mov cx, bx
mov al, bl

mov cx, [bx + si]
mov al, [bx + di]

mov [bx + di], cx
mov [bp + si], cl