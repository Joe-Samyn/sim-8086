
; ======================================================
; Performance test file for 8086 MOV instruction decoder
; Uses only instruction encodings currently supported:
;   - Register to register
;   - Memory to register (no disp, 8-bit disp, 16-bit disp)
;   - Direct address (non-accumulator only)
;   - Register to memory
;   - Immediate to register
;
; NOTE: No mov AX/AL, [direct] or mov [direct], AX/AL -
; those emit the accumulator short-form opcodes (A0-A3)
; which are not yet implemented in the decoder.
; ======================================================

bits 16

; --- Register to Register ---
mov cx, bx
mov al, bl
mov bx, cx
mov dx, ax
mov si, di
mov di, si
mov bp, sp
mov sp, bp
mov ah, ch
mov bh, dh
mov cl, dl
mov dh, bh
mov ax, dx
mov bx, si
mov cx, di
mov dx, bp
mov si, bx
mov di, cx
mov bp, dx
mov ax, cx
mov bx, dx
mov cx, si
mov dx, di
mov si, ax
mov di, bx
mov bp, cx
mov ax, si
mov bx, di
mov cx, bp
mov dx, ax
mov al, ah
mov bl, bh
mov cl, ch
mov dl, dh
mov ah, al
mov bh, bl
mov ch, cl
mov dh, dl
mov al, cl
mov bl, dl
mov cl, al
mov dl, bl
mov al, dl
mov bl, cl
mov cl, bl
mov dl, al
mov ah, bh
mov bh, ch
mov ch, dh
mov dh, ah
mov al, bl
mov bl, al
mov cl, dl
mov dl, cl

; --- Memory to Register, No Displacement ---
mov cx, [bx + si]
mov al, [bx + di]
mov dx, [bp + si]
mov bx, [bp + di]
mov ax, [si]
mov cx, [di]
mov dx, [bx]
mov si, [bx + si]
mov di, [bx + di]
mov bp, [bp + si]
mov ax, [bp + di]
mov bx, [si]
mov cx, [di]
mov bl, [bx + si]
mov cl, [bx + di]
mov dl, [bp + si]
mov ah, [bp + di]
mov bh, [si]
mov ch, [di]
mov dh, [bx]
mov ax, [bx + si]
mov bx, [bx + di]
mov cx, [bp + si]
mov dx, [bp + di]
mov si, [si]
mov di, [di]

; --- Memory to Accumulator --- 
mov ax, [7]
mov al, [1234]
mov ax, [243]
mov al, [2]

; --- Direct Address (no AX/AL - those use accumulator short form) ---
mov bx, [1234]
mov cx, [100]
mov dx, [200]
mov si, [300]
mov di, [400]
mov bp, [500]
mov bl, [2000]
mov cl, [3000]
mov dl, [4000]
mov ah, [5000]
mov bh, [6000]
mov ch, [7000]
mov dh, [8000]
mov bx, [10000]
mov cx, [11000]
mov dx, [12000]
mov si, [60000]
mov di, [59000]
mov bp, [58000]
mov bl, [57000]

; --- 8-bit Displacement ---
mov cx, [bx + si + 4]
mov ax, [bx + di + 8]
mov dx, [bp + si + 16]
mov bx, [bp + di + 32]
mov si, [si + 64]
mov di, [di + 127]
mov bp, [bx + 100]
mov al, [bx + si + 1]
mov bl, [bx + di + 2]
mov cl, [bp + si + 3]
mov dl, [bp + di + 5]
mov ah, [si + 6]
mov bh, [di + 7]
mov ch, [bx + 9]
mov dh, [bx + si + 10]
mov ax, [bx + di + 11]
mov bx, [bp + si + 12]
mov cx, [bp + di + 13]
mov dx, [si + 14]
mov si, [di + 15]
mov di, [bx + 17]
mov bp, [bx + si + 18]
mov ax, [bx + di + 19]
mov bx, [bp + si + 20]
mov cx, [bp + di + 21]
mov dx, [si + 22]
mov si, [di + 23]
mov al, [bx + 24]
mov bl, [bx + si + 25]
mov cl, [bx + di + 26]

; --- 16-bit Displacement ---
mov ax, [bx + di + 3254]
mov cx, [bx + si - 3254]
mov dx, [bp + si + 1000]
mov bx, [bp + di + 2000]
mov si, [si + 3000]
mov di, [di + 4000]
mov bp, [bx + 5000]
mov ax, [bx + si + 6000]
mov bx, [bx + di + 7000]
mov cx, [bp + si + 8000]
mov dx, [bp + di + 9000]
mov si, [si + 10000]
mov di, [di + 11000]
mov bl, [bx + si - 1000]
mov cl, [bx + di - 2000]
mov dl, [bp + si - 3000]
mov ah, [bp + di - 4000]
mov bh, [si + 500]
mov ch, [di + 600]
mov dh, [bx + 700]
mov ax, [bx + si - 500]
mov bx, [bx + di - 600]
mov cx, [bp + si - 700]
mov dx, [bp + di - 800]
mov si, [si + 15000]
mov di, [di + 20000]
mov bp, [bx + 25000]
mov ax, [bx + si + 30000]
mov bx, [bx + di - 30000]

; --- Register to Memory, No Displacement ---
mov [bx + di], cx
mov [bp + si], cl
mov [bx + si], ax
mov [bx + di], dx
mov [bp + si], si
mov [bp + di], di
mov [si], bx
mov [di], cx
mov [bx], dx
mov [bx + si], al
mov [bx + di], bl
mov [bp + si], cl
mov [bp + di], dl
mov [si], ah
mov [di], bh
mov [bx], ch
mov [bx + si], dh
mov [bx + di], ax
mov [bp + si], bx
mov [bp + di], cx

; --- Register to Memory, 8-bit Displacement ---
mov [bx + di + 8], cx
mov [bx + si - 17], ax
mov [bx + di + 1], dx
mov [bp + si + 2], bx
mov [bp + di + 3], si
mov [si + 4], di
mov [di + 5], bp
mov [bx + 6], ax
mov [bx + si + 7], al
mov [bx + di + 9], bl
mov [bp + si + 10], cl
mov [bp + di + 11], dl
mov [si + 12], ah
mov [di + 13], bh
mov [bx + 14], ch
mov [bx + si + 15], cx
mov [bx + di + 16], dx
mov [bp + si + 17], bx
mov [bp + di + 18], si
mov [si + 19], di

; --- Register to Memory, 16-bit Displacement ---
mov [bx + di + 3254], cx
mov [bx + si - 3254], ax
mov [bx + di + 1000], dx
mov [bp + si + 2000], bx
mov [bp + di + 3000], si
mov [si + 4000], di
mov [di + 5000], bp
mov [bx + 6000], ax
mov [bx + si + 7000], al
mov [bx + di + 8000], bl
mov [bp + si + 9000], cl
mov [bp + di + 10000], dl
mov [si + 11000], ah
mov [di + 12000], bh
mov [bx + 13000], ch
mov [bx + si - 1000], cx
mov [bx + di - 2000], dx
mov [bp + si - 3000], bx
mov [bp + di - 4000], si
mov [si - 5000], di

; --- Immediate to Register ---
mov ax, 3
mov bl, 9
mov cx, 5328
mov dx, 100
mov si, 200
mov di, 300
mov bp, 400
mov sp, 500
mov al, 1
mov bl, 2
mov cl, 3
mov dl, 4
mov ah, 5
mov bh, 6
mov ch, 7
mov dh, 8
mov ax, 1000
mov bx, 2000
mov cx, 3000
mov dx, 4000
mov si, 5000
mov di, 6000
mov bp, 7000
mov sp, 8000
mov ax, 32767
mov bx, 65535
mov cx, 256
mov dx, 512
mov si, 1024
mov di, 2048
mov al, 255
mov bl, 128
mov cl, 64
mov dl, 32
mov ah, 127
mov bh, 63
mov ch, 31
mov dh, 15
mov ax, 9999
mov bx, 8888
mov cx, 7777
mov dx, 6666

hlt
