

; ===================================================================
; Test: memory to accumulator move in 8086 assembly language
; This code moves a byte from memory to the AL register.
; ===================================================================

bits 16

mov al, [123]