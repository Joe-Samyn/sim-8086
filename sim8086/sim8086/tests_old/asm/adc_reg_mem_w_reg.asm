
; ===============================
; adc register or memory with  
; register using carry. 
; ===============================

bits 16

adc bx, [bx + si]

adc bx, cx
adc bl, cl