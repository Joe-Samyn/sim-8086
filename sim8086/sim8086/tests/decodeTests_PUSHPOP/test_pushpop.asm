

bits 16

push bx
push word [bx + si]
push word [bx + si + 29]

pop bx
pop word [bx + si]
pop word [bx + si + 29]