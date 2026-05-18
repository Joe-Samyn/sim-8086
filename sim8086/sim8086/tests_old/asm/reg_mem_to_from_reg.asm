

; =============================
; This file represents a basic 
; register to memory write in 
; Intel 8086 assembly.
; =============================


bits 16


; No displacement, read
mov cx, [bx + si]
mov al, [bx + di]

; No displacement, direct address
mov bx, [1234] 

; 8-bit displacement
mov cx, [bx + si + 4]

; 16-bit displacement
mov bx, [bx + di + 3254]

; Register to register 
mov cx, bx
mov al, bl
mov ax, si

; No displacement, write
mov [bx + di], cx
mov [bp + si], cl