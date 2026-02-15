default rel
section .text
global conv2d_asm

conv2d_asm:
    ;رجیستر هایی که بعد از انجام عملیات تابع نباید تغییر کنند را در استک پوش میکنیم تا در اخر انها را مجددا برگردانیم
    push r12
    push r13
    push r14
    push r15
    push rbx
    
    ;پارامتر های تابع
    ;rdi -> in
    ;rsi -> out
    ;rdx -> ker
    ;rcx -> w
    ;r8 -> h
    ;r9 -> k

    ; بارگزاری وزن ها از کرنل به صورت فلیپ شده 
    vbroadcastss ymm4, [rdx + 32] ;k8
    vbroadcastss ymm5, [rdx + 28] ;k7
    vbroadcastss ymm6, [rdx + 24] ;k6
    vbroadcastss ymm7, [rdx + 20] ;k5
    vbroadcastss ymm8, [rdx + 16] ;k4
    vbroadcastss ymm9, [rdx + 12] ;k3
    vbroadcastss ymm10,[rdx + 8]  ;k2
    vbroadcastss ymm11,[rdx + 4]  ;k1
    vbroadcastss ymm12,[rdx + 0]  ;k0


    mov r11, rcx ; w
    shl r11, 2   ; stride = w * 4

    xor r9, r9 ; y = 0 
.y_loop: ; حلقه y
    cmp r9, r8   ; y < h
    jge .done

    xor r10, r10 ;x=0
.x_loop: ; x حلقه
    cmp r10, rcx ; x < w
    jge .next_y

    ; مرزهای امن برای AVX
    cmp r9, 0
    je .scalar_pixel
    mov rax, r8
    dec rax
    cmp r9, rax ;y < h-1
    jge .scalar_pixel
    cmp r10, 0
    je .scalar_pixel
    mov rax, rcx
    sub rax, 9 ; به خاطر اینکه هشت تا هشت تا میخونیم و ضرب همزمان انجام میدیم 
    cmp r10, rax ; x <= w-۹
    jg .scalar_pixel

    
    mov rax, r9
    mul rcx
    add rax, r10 ; y * w + x
    lea r12, [rdi + rax*4] ;in اماده سازی برای خوندن از
    lea r15, [rsi + rax*4] ; out اماده سازی برای نوشتن در 

    vxorps ymm0, ymm0, ymm0 ; accumulator
    
    ; Row -1
    ; ادرس یه سطر بالا از موضعیت کنونی
    mov r13, r12
    sub r13, r11
    
    vmovups ymm1, [r13-4]
    vmulps  ymm1, ymm1, ymm4 ; k8 * وزن پیکسل
    vaddps  ymm0, ymm0, ymm1
    
    vmovups ymm1, [r13]
    vmulps  ymm1, ymm1, ymm5 ; k7 * وزن پیکسل
    vaddps  ymm0, ymm0, ymm1
    
    vmovups ymm1, [r13+4]
    vmulps  ymm1, ymm1, ymm6 ; k6 * وزن پیکسل
    vaddps  ymm0, ymm0, ymm1
    
    ; Row 0
    ; سطر موقعیت کنونی
    vmovups ymm1, [r12-4]
    vmulps  ymm1, ymm1, ymm7 ; k5 * وزن پیکسل
    vaddps  ymm0, ymm0, ymm1
    
    vmovups ymm1, [r12]
    vmulps  ymm1, ymm1, ymm8 ; k4 * وزن پیکسل
    vaddps  ymm0, ymm0, ymm1
    
    vmovups ymm1, [r12+4]
    vmulps  ymm1, ymm1, ymm9 ; k3 * وزن پیکسل
    vaddps  ymm0, ymm0, ymm1
    
    ; Row +1
    ;یک سطربالاتر از سطر کنونی
    lea r13, [r12+r11]
    
    vmovups ymm1, [r13-4]
    vmulps  ymm1, ymm1, ymm10 ; k2 * وزن پیکسل
    vaddps  ymm0, ymm0, ymm1
    
    vmovups ymm1, [r13]
    vmulps  ymm1, ymm1, ymm11 ; k1 * وزن پیکسل
    vaddps  ymm0, ymm0, ymm1
    
    vmovups ymm1, [r13+4]
    vmulps  ymm1, ymm1, ymm12 ; k0 * وزن پیکسل
    vaddps  ymm0, ymm0, ymm1

    vmovups [r15], ymm0 ; out نوشتن روی 
    add r10, 8 ; x = x + 8
    jmp .x_loop

    ; برای گوشه ها و پیکسل های دست نخورده
.scalar_pixel:
    ;  همون منطق کد سی و ضرب تکی تکی 
    mov rax, r9
    mul rcx
    add rax, r10 ;موقعیت کنونی = y * w + x

    movss xmm0, [rdi + rax*4]
    movss [rsi + rax*4], xmm0 
    inc r10
    jmp .x_loop

.next_y:
    inc r9
    jmp .y_loop

.done:
    vzeroupper ;avx از sse برای بازگشت به حالت 
    pop rbx
    pop r15
    pop r14
    pop r13
    pop r12
    ret

section .note.GNU-stack noalloc noexec nowrite progbits  ; به کامپایلر و لینکر میگیم که استک اجرایی نیست و فقط برای دیتا هست تا برنامه امن اجرا بشه