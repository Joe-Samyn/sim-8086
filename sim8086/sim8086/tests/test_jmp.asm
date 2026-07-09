

bits 16

jmp test_function
    add cx, 18
test_function:
    mov bx, cx
    jmp test_function_2

test_function_2:
    mov cx, dx
