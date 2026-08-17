
bits 16

mov bx, 120
mov si, 20
mov [140], 0x15
mov [bx + 5], 50
mov cx, [bx + 5]

mov dx, [bx + si]