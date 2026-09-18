; Author: Joe Samyn
; Program: Creates a color gradient visual in memory using RGBA format.
; Note: In the simulator we are running this program on, all registers are initialized to 0 by default. So, its safe to assume
; the registers already are zeroed.

bits 16

; Initialize the variables
mov cx, 64
mov bp, 256

outer_loop:
    mov dx, 64

    inner_loop:
        mov byte [bp], bl
        mov byte [bp + 1], 0
        mov byte [bp + 2], al
        mov byte [bp + 3], 255

        add bp, 4
        add al, 1

        sub dx, 1
        jnz inner_loop

    add bl, 1
    loop outer_loop
