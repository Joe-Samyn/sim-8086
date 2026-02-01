

; =============================
; This file represents a basic 
; register to memory write in 
; Intel 8086 assembly.
; =============================


bits 16

mov cx, bx
mov al, bl

; No displacement, read
mov cx, [bx + si]
mov al, [bx + di]

; 8-bit displacement
mov cx, [bx + si + 4]

; 16-bit displacement
mov ax, [bx + di + 3254]

; No displacement, write
mov [bx + di], cx
mov [bp + si], cl
