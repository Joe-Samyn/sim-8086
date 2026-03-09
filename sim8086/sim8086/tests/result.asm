

bits 16


MOV CX, BX
MOV AL, BL
MOV CX, [BX + SI]
MOV AL, [BX + DI]
MOV BX, [1234]
MOV CX, [BX + SI - 252]
MOV AX, [BX + DI + 3254]
MOV CX, [BX + SI + -3254]
MOV [BX + DI], CX
MOV [BX + DI - 248], CX
MOV [BX + SI - 17], BX
MOV [BX + DI], CX
MOV [BP + SI], CL
MOV AX, 3
MOV BL, -247
MOV CX, 5328
MOV [BX + DI], -244
MOV BX, 12
MOV AL, [12]
MOV AL, [1234]
