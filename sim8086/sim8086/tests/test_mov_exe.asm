
bits 16

mov bx, 1024
mov bl, 17
mov bh, 64

mov cx, bx
mov dl, bh
mov al, cl
mov dh, bh

mov word [120], 0x5577
mov bh, [120]