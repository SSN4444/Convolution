#include "conv.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

double now();
float* readPGM(const char*, int*, int*);
void writePGM(const char*, float*, int, int);

void patternRecognition(const char* folder, int count,const char* kernel){
    int patternFounded_c = 0;
    int patternFounded_asm = 0;
    double time_c = 0.0;
    double time_asm = 0.0;
    //خواندن کرنل و آماده‌سازی
    int w_k, h_k;
    float* ker = readPGM(kernel, &w_k, &h_k);
    int k = w_k;
    float sum_k = 0.0f;
        
    // معکوس کردن کرنل (تا الگو مورد نظر  سفید شود) و محاسبه مجموع
    for(int i = 0; i < w_k * h_k; i++) {
        ker[i] = 255.0f - ker[i];
        sum_k += ker[i];
    }
    float avg_k = sum_k / (k * k);

    for(int i = 0; i < w_k * h_k; i++) {
        ker[i] -= avg_k; // Zero-mean کردن
    }

    for(int id=1; id<=count; id++)
    {
        char name[256];
        sprintf(name, "%s/input%d.pgm", folder, id);

        // خواندن تصویر و معکوس کردن (Invert)
        int w,h;
        float* in = readPGM(name, &w, &h);
        for (int i = 0 ; i < w*h ; i++) {
            in[i] = 255.0f - in[i]; 
        }

        //  اجرای کانولوشن (با نسخه اسمبلی یا C)
        float* out_c = calloc(w*h, sizeof(float));
        double t1 = now();
        conv2d_c(in, out_c, ker, w, h, w_k);
        // writePGM("output_c.pgm", out_c, w, h);


        //  محاسبه انرژی خودِ الگو (Template Energy)
        float energy_k = 0;
        for(int i = 0; i < w_k * h_k; i++) {
            energy_k += ker[i] * ker[i];
        }

        float max_ncc = -1.0f;
        // int best_x = 0, best_y = 0;

        for (int y = 0; y < h - h_k; y++) {
            for (int x = 0; x < w - w_k; x++) {
                //  محاسبه انرژی تکه‌ای از تصویر که زیر الگو قرار دارد (Window Energy)
                float energy_win = 0;
                for (int ky = 0; ky < h_k; ky++) {
                    for (int kx = 0; kx < w_k; kx++) {
                        float pixel = in[(y + ky) * w + (x + kx)];
                        energy_win += pixel * pixel;
                    }
                }

                //  مقدار خروجی کانولوشن در این نقطه
                float val = out_c[y * w + x];

                //  فرمول NCC: همبستگی تقسیم بر جذر ضرب انرژی‌ها
                float denom = sqrtf(energy_k * energy_win);
                float ncc = (denom > 0) ? (val / denom) : 0;

                if (ncc > max_ncc) {
                    max_ncc = ncc;
                    // best_x = x;
                    // best_y = y;
                }
            }
        }

        //  نمایش نتیجه نهایی
        float confidence = max_ncc * 100.0f;
        if (confidence < 0) confidence = 0;
        if (confidence > 100) confidence = 100;
        if (confidence > 85.0f) {// با دقت ۸۵ درصد
            patternFounded_c ++;
        }
        double t2 = now();
        time_c += t2-t1;


        double t3 = now();
        //  ASM 

        //zero paddings
        int pad = k/2;
        int h2 = h + 2*pad;
        // تعداد سطر ها بر ۸ بخش پذیر باشد تا در هشت تا هشت تا خوندن اعداد به مشکل نخوریم
        int w2 = w + 2*pad;
        int w_pad = ((w2 + 7) / 8) * 8;
        w_pad += 2 * pad;

        float* in_pad  = calloc(w_pad*h2, sizeof(float));
        float* out_pad = calloc(w_pad*h2, sizeof(float));
        float* out_asm = calloc(w*h, sizeof(float));
        //padding
        for(int y=0; y<h; y++){
            memcpy(in_pad + (y+pad)*w_pad + pad,in + y*w,w*sizeof(float)); //(تعداد عدد خوانده شده,ادرس مبدا,ادرس مقصد)
        }

        //اجرای convolution در asm
        
        conv2d_asm(in_pad, out_pad, ker, w_pad, h2, k);

        // استخراج تصویر اصلی از تصویر پدینگ شده
        for(int y=0; y<h; y++){
            memcpy(out_asm + y*w,
            out_pad + (y+pad)*w_pad + pad,
            w*sizeof(float));
        }
        //  محاسبه انرژی خودِ الگو (Template Energy)
        float energy_k = 0;
        for(int i = 0; i < w_k * h_k; i++) {
            energy_k += ker[i] * ker[i];
        }

        float max_ncc = -1.0f;
        // int best_x = 0, best_y = 0;

        for (int y = 0; y < h - h_k; y++) {
            for (int x = 0; x < w - w_k; x++) {
                //  محاسبه انرژی تکه‌ای از تصویر که زیر الگو قرار دارد (Window Energy)
                float energy_win = 0;
                for (int ky = 0; ky < h_k; ky++) {
                    for (int kx = 0; kx < w_k; kx++) {
                        float pixel = in[(y + ky) * w + (x + kx)];
                        energy_win += pixel * pixel;
                    }
                }

                //  مقدار خروجی کانولوشن در این نقطه
                float val = out_asm[y * w + x];

                //  فرمول NCC: همبستگی تقسیم بر جذر ضرب انرژی‌ها
                float denom = sqrtf(energy_k * energy_win);
                float ncc = (denom > 0) ? (val / denom) : 0;

                if (ncc > max_ncc) {
                    max_ncc = ncc;
                    // best_x = x;
                    // best_y = y;
                }
            }
        }

        //  نمایش نتیجه نهایی
        float confidence = max_ncc * 100.0f;
        if (confidence < 0) confidence = 0;
        if (confidence > 100) confidence = 100;
        if (confidence > 85.0f) { // با دقت ۸۵ درصد
            patternFounded_asm ++;
        }
        double t4 = now();
        time_asm += t4-t3;



        // printf("Max NCC Score: %.4f\n", max_ncc);
        // printf("Result: %s (Confidence: %.2f%%) at [%d, %d]\n", 
        //     (confidence > 85.0f) ? "FOUND" : "NOT FOUND", confidence, best_x, best_y);
        // پیدا کردن مقدار ماکزیمم واقعی که C تولید کرده
        // float absolute_max = -1e20f; 
        // for (int i = 0; i < w * h; i++) {
        //     if (out_c[i] > absolute_max) absolute_max = out_c[i];
        // }

        // // ۲. تمیز کردن تصویر: فقط قله‌ها سفید، بقیه سیاه مطلق
        // for (int i = 0; i < w * h; i++) {
        //     // اگر مقدار پیکسل بیشتر از ۹۵٪ِ ماکزیمم بود (یعنی خودِ الگوست)
        //     if (out_c[i] >= absolute_max * 0.95f && absolute_max > 0) {
        //         out_c[i] = 255.0f; // سفید درخشان
        //     } else {
        //         out_c[i] = 0.0f;   // سیاه مطلق
        //     }
        // }


        // writePGM("out_centerOfPattern.pgm", out_c, w, h);

    }

    //  گزارش 

    printf("REPORT \n");

    printf("C   total time  : %f sec\n", time_c);
    printf("ASM total time  : %f sec\n", time_asm);
    printf("Speedup         : %.2fx\n", time_c/time_asm);

    printf("PATTERN FOUND IN %d FROM %d PICTURE (C)\n", found_c, count);
    printf("PATTERN FOUND IN %d FROM %d PICTURE (ASM)\n", found_asm, count);

}