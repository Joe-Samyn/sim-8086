
; ================================
; adc immediate to register or 
; memory using carry.
; ================================

bits 16

adc bl, 12
adc cx, 1000

adc [bp + di], 17
adc [bp + di + 7], 26
adc [bx - 1234], 45

adc word [bp + di], 260
adc word [bp + di], 8
adc word [bp + di], -8

adc word [bp + di + 7], -27