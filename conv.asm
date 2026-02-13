default rel
section .text
global conv2d_asm

; rdi=in

; rsi=out
; rdx=ker
; rcx=w
; r8=h

conv2d_asm:

    push r12
    push r13
    push r14
    push r15

    mov r12, rdi        ; in
    mov r13, rsi        ; out
    mov r14, rdx        ; ker
    mov r15, rcx        ; w
    mov r10, r8         ; h

    ; interior width limit = w-1-7
    mov rbx, r15
    sub rbx, 8
    sub rbx, 1          ; rbx = max x

    mov r9, 1           ; y = 1

; ================= y loop =================
.y_loop:
    cmp r9, r10
    jge .done
    cmp r9, 1
    je .y_ok
    cmp r9, r10
.y_ok:

    mov r11, 1          ; x = 1

; ================= x loop (SIMD safe only) =================
.x_loop:
    cmp r11, rbx
    jg .scalar_tail


    ; idx = y*w + x
    mov rax, r9
    imul rax, r15
    add rax, r11

    lea r8,  [r12 + rax*4]   ; in ptr
    lea rcx, [r13 + rax*4]   ; out ptr

    vxorps ymm0, ymm0, ymm0

    ; row bytes
    mov rdx, r15
    shl rdx, 2

; ----------- 9 taps (3x3) -------------

; row -1
    mov rsi,r8
    sub rsi,rdx
    sub rsi,4
    vmovups ymm1, [rsi]
    vbroadcastss ymm2, [r14+0]
    vmulps ymm1, ymm1, ymm2
    vaddps ymm0, ymm0, ymm1
    
    mov rsi,r8
    sub rsi,rdx
    vmovups ymm1, [rsi]
    vbroadcastss ymm2, [r14+4]
    vmulps ymm1, ymm1, ymm2
    vaddps ymm0, ymm0, ymm1

    mov rsi,r8
    sub rsi,rdx
    add rsi,4
    vmovups ymm1, [rsi]
    vbroadcastss ymm2, [r14+8]
    vmulps ymm1, ymm1, ymm2
    vaddps ymm0, ymm0, ymm1

; row 0
    lea rsi, [r8-4]
    vmovups ymm1, [rsi]
    vbroadcastss ymm2, [r14+12]
    vmulps ymm1, ymm1, ymm2
    vaddps ymm0, ymm0, ymm1

    vmovups ymm1, [r8]
    vbroadcastss ymm2, [r14+16]
    vmulps ymm1, ymm1, ymm2
    vaddps ymm0, ymm0, ymm1

    lea rsi, [r8+4]
    vmovups ymm1, [rsi]
    vbroadcastss ymm2, [r14+20]
    vmulps ymm1, ymm1, ymm2
    vaddps ymm0, ymm0, ymm1

; row +1
    lea rsi, [r8+rdx-4]
    vmovups ymm1, [rsi]
    vbroadcastss ymm2, [r14+24]
    vmulps ymm1, ymm1, ymm2
    vaddps ymm0, ymm0, ymm1

    lea rsi, [r8+rdx]
    vmovups ymm1, [rsi]
    vbroadcastss ymm2, [r14+28]
    vmulps ymm1, ymm1, ymm2
    vaddps ymm0, ymm0, ymm1

    lea rsi, [r8+rdx+4]
    vmovups ymm1, [rsi]
    vbroadcastss ymm2, [r14+32]
    vmulps ymm1, ymm1, ymm2
    vaddps ymm0, ymm0, ymm1

; store
    vmovups [rcx], ymm0

    add r11, 8
    jmp .x_loop

.scalar_tail:


.next_row:
    inc r9
    jmp .y_loop

.done:
    vzeroupper

    pop r15
    pop r14
    pop r13
    pop r12
    ret
    
