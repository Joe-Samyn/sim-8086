bits 16

    mov cx, 3 
    mov bx, 100

loop_one:
    add bx, 100
    sub cx, 1 
    jnz loop_one 