
; ===============================
; Add immediate to accumulator 
; with carry. 
; ===============================

bits 16

adc word ax, 300
adc ax, 12

adc al, 220

hlt