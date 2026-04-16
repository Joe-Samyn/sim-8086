; ==========================
; This 8086 assembly code 
; performs an immediate to 
; register move operation. 
; ==========================

bits 16

mov byte [bx + di], 12
mov bx, 12
mov ax, [12]
mov al, [1234]