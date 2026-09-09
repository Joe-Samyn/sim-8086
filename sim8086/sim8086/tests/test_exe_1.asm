bits 16

    mov dx, 10
    mov bp, 1000

init_loop:
    mov [bp + si], si
    add si, 2
    cmp dx, si 
    jnz init_loop 

    mov si, 0
sum_loop:
    add bx, [bp + si]
    add si, 2
    cmp dx, si 
    jnz sum_loop 


