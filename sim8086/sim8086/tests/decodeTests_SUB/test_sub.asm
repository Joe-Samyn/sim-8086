bits 16

sub cx, bx
sub [bx + si], cx
sub [bx + si + 5], dx
sub bx, 18
sub word [bx + si + 17], 1024
sub ax, 28

sbb cx, bx
sbb [bx + si], cx
sbb [bx + si + 5], dx
sbb bx, 18
sbb word [bx + si + 17], 1024
sbb ax, 28