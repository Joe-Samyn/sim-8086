

bits 16

xchg bx, dx
xchg cx, [bx + si]
xchg cx, [bx + si + 1024]
xchg cx, [bx + si - 37]
xchg ax, cx

in ax, 9
in al, 122

in ax, dx
in al, dx

out 9, ax
out 122, al
out dx, ax
out dx, al