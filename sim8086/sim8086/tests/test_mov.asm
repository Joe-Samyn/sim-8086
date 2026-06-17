
;---------------------------------------
; Simply variations of the Intel 8086
; MOV instruction used to verify decode
; logic. 
;---------------------------------------

bits 16

; Register to register moves
mov ax, bx
mov cx, dx
mov si, bp