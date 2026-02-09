
; ======================================================
; Test file for all MOV instructions in Intel 8086 
; assembly language. 
; ======================================================

bits 16

mov cx, bx
mov al, bl

; No displacement, read
mov cx, [bx + si]
mov al, [bx + di]

; No displacement, direct address
mov bx, [1234]

; 8-bit displacement
mov cx, [bx + si + 4]

; 16-bit displacement
mov ax, [bx + di + 3254]

; No displacement, write
mov [bx + di], cx
mov [bp + si], cl

; Immediate to Registers
mov ax, 3
mov bl, 9
mov cx, 5328