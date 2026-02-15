#pragma once   // جلوگیری از چندبار include شدن فایل هدر (جلوگیری از خطای تکرار تعریف)

// توابع کانولوشن دوبعدی تصویر
// هر دو تابع یک کار انجام می‌دهند اما:
// conv2d_c   -> پیاده‌سازی ساده و مرجع با C (کندتر)
// conv2d_asm -> پیاده‌سازی بهینه با Assembly + AVX (سریع‌تر)

// in: تصویر ورودی (آرایه 1بعدی به صورت row-major)
// out: تصویر خروجی
// kernel: کرنل k×k
// w,h: عرض و ارتفاع تصویر
// k: اندازه کرنل

void conv2d_c(float* in, float* out, float* kernel,
              int w, int h, int k);


void conv2d_asm(float* in, float* out, float* kernel,
                int w, int h, int k);
