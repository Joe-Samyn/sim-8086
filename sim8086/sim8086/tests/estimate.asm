bits 16

mov bx, 1000                    ; 4
mov bp, 2000                    ; 4
mov si, 3000                    ; 4
mov di, 4000                    ; 4

mov cx, [bp + di]               ; 8 + 7 = 15
mov [bx + si], cx               ; 9 + 7 = 16

mov cx, [bp + si]               ; 8 + 8 = 16
mov [bx + di], cx               ; 9 + 8 = 17

mov cx, [bp + di + 1000]        ; 8 + 11 = 19
mov [bx + si + 1000], cx        ; 9 + 11 = 20

mov cx, [bp + si + 1000]        ; 8 + 12 = 20
mov [bx + di + 1000], cx        ; 9 + 12 = 21

add dx, [bp + si + 1000]        ; 9 + 12 = 21

add word [bp + si], 76          ; 17 + 8 = 25

add dx, [bp + si + 1001]        ; 9 + 12 + (1 * 4) = 25
add [di + 999], dx              ; 16 + 9 + (2 * 4) = 33
add word [bp + si], 75          ; 17 + 8 = 25

                                ; total: 289 cycles
