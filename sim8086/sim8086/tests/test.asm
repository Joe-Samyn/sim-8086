
bits 16
    JMP $+2
    MOV BX, CX
    JMP $+2
    MOV CX, DX
    JMP $-8
    JMP $+490
    JZ $-11
    JNG $-13
    JNGE $-15
    JNAE $-17
    JNA $-19
    JPE $-21
    JO $-23
    JS $-25
    JNZ $-27
    JGE $-29
    JG $-31
    JAE $-33
    JA $-35
    JPO $-37
    JNO $-39
    JNS $-41
    JMP BX
    JMP word [BX + SI]
    JMP word [BX + SI + 17]
    LOOP $-50
    LOOPZ $-52
    LOOPNZ $-54
    JCXZ $-56
    RET 