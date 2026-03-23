bits 16

MOV word CX, [BX + SI]
MOV byte AL, [BX + DI]
MOV word BX, [1234]
MOV word CX, [BX + SI + 4]
MOV word BX, [BX + DI + 3254]
MOV word CX, BX
MOV byte AL, BL
MOV word AX, SI
MOV word [BX + DI], CX
MOV byte [BP + SI], CL
hlt