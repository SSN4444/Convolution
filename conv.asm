;پیاده سازی تابع کانولوشن با زبان اسمبلی و استفاده از دستورات برداری SIMD
default rel ; فعال‌سازی آدرس‌دهی نسبی (relative)
section .text
global conv2d_asm

conv2d_asm:

    ;رجیستر هایی که بعد از انجام عملیات تابع نباید تغییر کنند را در استک پوش میکنیم تا در اخر انها را مجددا برگردانیم
    push r12
    push r13
    push r14
    push r15
    push rbx

    ; پارامترها: rdi=in, rsi=out, rdx=ker, rcx=w, r8=h, r9=k
    
    mov r14, r9         ; r14 = k (اندازه کرنل)
    shr r14, 1          ; r14 = pad = k / 2 (برای شروع از پیکسل درست)

    ; شروع حلقه سطرهای تصویر (y)
    mov rbx, r14        ; rbx = y = pad
.y_loop:
    mov rax, r8 
    sub rax, r14        ; limit = h - pad
    cmp rbx, rax 
    jge .done   ;اگر به سطر اخر رسیدیم کار تمام است

    ; شروع حلقه ستون‌های تصویر (x)
    mov r12, r14        ; r12 = x = pad
.x_loop:
    mov rax, rcx
    sub rax, r14
    sub rax, 8          ;  (w - pad - 8)چک برای پردازش 8 پیکسل همزمان
    cmp r12, rax        ; rax -> اخرین نقطه ایست که میتوان بعد از ان ۸ پیکسل را مورد پردازش قرار داد
    jg .scalar_pixel    ; اگر به انتهای سطر نزدیک شدیم، گذر میکنیم 

    vxorps ymm0, ymm0, ymm0 ; Accumulator (sum) برای 8 پیکسل خروجی

    ;  حلقه روی سطرهای کرنل (ky) 
    xor r13, r13        ; r13 = ky = 0
.ky_loop:
    ; محاسبه آدرس سطر در تصویر: (y + ky - pad) * w 

    ; محاسبه آدرس سطر در کرنل به صورت فلیپ شده -> (k-1-ky) * k
    mov r15,r9
    dec r15
    sub r15, r13
    imul r15, r9        

    ;  حلقه روی ستون‌های کرنل (kx) 
    xor r10, r10        ; r10 = kx = 0
.kx_loop:
    ;  پخش کردن وزن فعلی کرنل weight = ker[ky * k + kx]
    mov rax, r15
    add rax, r9
    dec rax
    sub rax,r10 ;rax = (k-1-ky)*k + (k-1-kx)
    vbroadcastss ymm2, [rdx + rax*4] ;خواندن وزن از کرنل

    ; خواندن 8 پیکسل از تصویر: image[((y+ky-pad)*w) + (x+kx-pad)]
    mov rax, rbx  ; rax = y = rbx
    add rax, r13   ; rax = y + ky
    sub rax, r14    ;rax = y + ky - pad 
    imul rax, rcx    ;rax = (y + ky - pad) * w -> سطر  
    add rax, r12        ; اضافه کردن x
    add rax, r10        ; اضافه کردن kx
    sub rax, r14        ; کسر کردن pad 
    ;rax = iy * w - ix
    
    vmovups ymm1, [rdi + rax*4] ;خواندن از ارایه ورودی 
    
    ;  ضرب و جمع در یک سیکل پرداشی 
    vfmadd231ps ymm0, ymm1, ymm2 ; ymm0 = (ymm1 * ymm2) + ymm0

    inc r10
    cmp r10, r9         ; kx < k
    jl .kx_loop

    inc r13
    cmp r13, r9         ; ky < k
    jl .ky_loop

    ; ذخیره نتیجه 8 پیکسل در خروجی
    mov rax, rbx ;rax = y
    imul rax, rcx ;rax = y * w
    add rax, r12 ;rax  = y * w  + x
    vmovups [rsi + rax*4], ymm0

    add r12, 8          ; حرکت 8 پیکسل به جلو در عرض تصویر
    jmp .x_loop

.scalar_pixel:
    ; اینجا همه ی پیکسل ها به خاطر پدینگ صفر هستند پس همه را رد میکنیم تا به سطر بعدی برسیم
    inc r12
    mov rax, rcx ;rax = w
    sub rax, r14 ; rax = w - pad
    cmp r12, rax ; x < w - pad
    jl .scalar_pixel

.next_y:
    inc rbx             ; y++
    jmp .y_loop

.done:
    vzeroupper ; برای رفتن از حالت AVX به SSE 

    ;برگردانن پارامتر هایی کگه در ابتدا در استک ذخیره کرده بودیم
    pop rbx
    pop r15
    pop r14
    pop r13
    pop r12
    ret

section .note.GNU-stack noalloc noexec nowrite progbits ; به کامپایلر و لینکر میگیم که استک اجرایی نیست و فقط برای دیتا هست تا برنامه امن اجرا بشه