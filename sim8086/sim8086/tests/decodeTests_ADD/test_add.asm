; ------------------------------------------------------
; Sample ADD instructions for the Intel 8086 processor
; ------------------------------------------------------
bits 16

add cx, bx
add [bx + si], cx
add [bx + si + 5], dx
add bx, 18
add word [bx + si + 17], 1024
add ax, 28

adc bx, 33

adc cx, bx
adc [bx + si], cx
adc [bx + si + 5], dx
adc bx, 18
adc word [bx + si + 17], 1024
adc ax, 28