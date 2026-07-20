

bits 16


fun_one:
    jmp label
    add cx, 18
label:
    mov bx, cx
    jmp test_function_2

test_function_2:
    mov cx, dx
    jmp fun_one
    jmp 500
    jz label
    jng label
    jnge label
    jnae label
    jna label
    jpe label
    jo label
    js label
    jnz label
    jge label
    jg label
    jae label
    ja label
    jpo label
    jno label
    jns label

    jmp bx
    jmp [bx + si]
    jmp [bx + si + 17]

    loop label
    loopz label
    loopnz label

    jcxz label
    ret
