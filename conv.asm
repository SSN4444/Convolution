default rel
section .text
global conv2d_asm

conv2d_asm:
    push r12
    push r13
    push r14
    push r15
    push rbx

    ; ۱. بارگذاری وزن‌ها (طبق همون ترتیبی که خطا رو کم کرد)
    vbroadcastss ymm4, [rdx + 32] 
    vbroadcastss ymm5, [rdx + 28] 
    vbroadcastss ymm6, [rdx + 24] 
    vbroadcastss ymm7, [rdx + 20] 
    vbroadcastss ymm8, [rdx + 16] 
    vbroadcastss ymm9, [rdx + 12] 
    vbroadcastss ymm10,[rdx + 8]  
    vbroadcastss ymm11,[rdx + 4]  
    vbroadcastss ymm12,[rdx + 0]  

    mov r11, rcx ; w
    shl r11, 2   ; stride = w * 4

    xor r9, r9   ; y = 0
.y_loop:
    cmp r9, r8   ; y < h
    jge .done

    xor r10, r10 ; x = 0
.x_loop:
    cmp r10, rcx ; x < w
    jge .next_y

    ; مرزهای امن برای AVX
    cmp r9, 0
    je .scalar_pixel
    mov rax, r8
    dec rax
    cmp r9, rax
    jge .scalar_pixel
    cmp r10, 0
    je .scalar_pixel
    mov rax, rcx
    sub rax, 9
    cmp r10, rax
    jg .scalar_pixel

    ; --- بخش سریع AVX (بدون FMA) ---
    mov rax, r9
    mul rcx
    add rax, r10
    lea r12, [rdi + rax*4] 
    lea r15, [rsi + rax*4] 

    vxorps ymm0, ymm0, ymm0 ; accumulator
    
    ; Row -1
    mov r13, r12
    sub r13, r11
    
    vmovups ymm1, [r13-4]
    vmulps  ymm1, ymm1, ymm4
    vaddps  ymm0, ymm0, ymm1
    
    vmovups ymm1, [r13]
    vmulps  ymm1, ymm1, ymm5
    vaddps  ymm0, ymm0, ymm1
    
    vmovups ymm1, [r13+4]
    vmulps  ymm1, ymm1, ymm6
    vaddps  ymm0, ymm0, ymm1
    
    ; Row 0
    vmovups ymm1, [r12-4]
    vmulps  ymm1, ymm1, ymm7
    vaddps  ymm0, ymm0, ymm1
    
    vmovups ymm1, [r12]
    vmulps  ymm1, ymm1, ymm8
    vaddps  ymm0, ymm0, ymm1
    
    vmovups ymm1, [r12+4]
    vmulps  ymm1, ymm1, ymm9
    vaddps  ymm0, ymm0, ymm1
    
    ; Row +1
    lea r13, [r12+r11]
    
    vmovups ymm1, [r13-4]
    vmulps  ymm1, ymm1, ymm10
    vaddps  ymm0, ymm0, ymm1
    
    vmovups ymm1, [r13]
    vmulps  ymm1, ymm1, ymm11
    vaddps  ymm0, ymm0, ymm1
    
    vmovups ymm1, [r13+4]
    vmulps  ymm1, ymm1, ymm12
    vaddps  ymm0, ymm0, ymm1

    vmovups [r15], ymm0
    add r10, 8
    jmp .x_loop

.scalar_pixel:
    ; ... (همون منطق کپی ساده) ...
    mov rax, r9
    mul rcx
    add rax, r10
    movss xmm0, [rdi + rax*4]
    movss [rsi + rax*4], xmm0 
    inc r10
    jmp .x_loop

.next_y:
    inc r9
    jmp .y_loop

.done:
    vzeroupper
    pop rbx
    pop r15
    pop r14
    pop r13
    pop r12
    ret