
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
mov bx, [bx + di]
mov dx, [39]
mov cx, [-5]

; Effective address calculation with 8-bit displacement moves
mov bx, [bx + si + 12]
mov cx, [bp + di + 115]
mov dx, [bp - 2]
mov cx, [bp + di - 115]

; Effective address calculation with 16-bit displacement moves
mov cx, [bp + di + 1024]
mov dx, [bx + si - 520]
mov bx, [si + 1040]