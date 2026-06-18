
;---------------------------------------
; Simply variations of the Intel 8086
; MOV instruction used to verify decode
; logic. 
;---------------------------------------

bits 16

; Full register to register moves 
mov ax, bx
mov cx, dx
mov si, bp

; Low bit register to register moves 
mov al, bl
mov cl, dl

; Effective address calculation moves 
mov cx, [bx + si]