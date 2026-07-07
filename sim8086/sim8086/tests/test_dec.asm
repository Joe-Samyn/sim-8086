

bits 16
dec bx
dec word [bx + si]
dec word [bx + si + 29]
neg bx
inc bx
inc word [bx + si]
inc byte [bx + si + 29]