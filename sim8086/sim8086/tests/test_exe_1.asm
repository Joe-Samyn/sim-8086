bits 16

    mov cx, 3 
    mov bx, 3
    cmp cx, bx
    jz loop_two

loop_one:
    add bx, 100
    sub cx, 1 
    jnz loop_one 

loop_two:
    add bx, 10
    sub cx, 1
    jnz loop_two