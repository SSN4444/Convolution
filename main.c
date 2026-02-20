#include "conv.h" // اعلان پروتوتایپ توابع convolution برای استفاده در فایل‌های دیگر
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "object_recognition.h"// اعلان پروتوتایپ 
#include "pattern_recognition.h" // اعلان پروتوتایپ 

double now(); //پروتوتایپ تابع مربوط به زمان در فایل timer.c


// خواندن تصویر PGM و تبدیل آن به آرایه float
float* readPGM(const char* name, int* w, int* h)
{
    FILE* f = fopen(name, "rb");   // باز کردن فایل باینری
    if (!f) { 
        printf("%s\n",name);
        printf("Cannot open input\n"); exit(1); 
    }

    char m[3];

    // خواندن نوع فایل (باید P5 باشد)
    if (fscanf(f, "%2s", m) != 1) exit(1);
    if (strcmp(m,"P5") != 0) exit(1);

    // خواندن عرض و ارتفاع
    if (fscanf(f, "%d %d", w, h) != 2) exit(1);

    int max;
    if (fscanf(f, "%d", &max) != 1) exit(1);

    fgetc(f);  // رد شدن از newline

    // خواندن داده‌های خام تصویر
    unsigned char* tmp = malloc((*w)*(*h));
    size_t n = fread(tmp, 1, (*w)*(*h), f);
    if (n != (*w)*(*h)) exit(1);

    fclose(f);

    // تبدیل unsigned char → float
    float* img = malloc(sizeof(float)*(*w)*(*h));
    for(int i=0;i<(*w)*(*h);i++)
        img[i] = (float)tmp[i];

    free(tmp);

    return img;
}




// ذخیره آرایه float به صورت تصویر PGM
void writePGM(const char* name, float* img, int w, int h)
{
    FILE* f = fopen(name, "wb");

    fprintf(f,"P5\n%d %d\n255\n", w, h); // هدر فایل

    for(int i=0;i<w*h;i++)
    {
        float v = img[i];

        // محدود کردن به بازه 0..255
        if(v < 0) v = 0;
        if(v > 255) v = 255;

        unsigned char c = (unsigned char)(v);
        fwrite(&c,1,1,f);
    }

    fclose(f);
}


int main(int argc, char *argv[])
{   

    if (!strcmp("object_recognition",argv[1])) {
        objectRecognition("inputs",400);
        return 0;
    }
    if (!strcmp("pattern_recognition",argv[1])) {
        patternRecognition("inputs2",150,argv[2]);
        return 0;
    }

    if (!strcmp("ker",argv[1])) {
        // ۱. خواندن تصویر و معکوس کردن (Invert)
        int w,h;
        // ساخت ادرس تصویر ورودی
        char base[256] = "inputs/";
        strcat(base, argv[2]);     
        char* input = base;
        float* in = readPGM(input, &w, &h);
        for (int i = 0 ; i < w*h ; i++) {
            in[i] = 255.0f - in[i]; 
        }

        // ۲. خواندن کرنل و آماده‌سازی (فقط یک بار)
        int w_k, h_k;
        float* ker = readPGM("ker.pgm", &w_k, &h_k);
        float sum_k = 0.0f;
        
        // معکوس کردن کرنل (تا پلاس سفید شود) و محاسبه مجموع
        for(int i = 0; i < w_k * h_k; i++) {
            ker[i] = 255.0f - ker[i];
            sum_k += ker[i];
        }
        float avg_k = sum_k / (w_k * h_k);

        float perfect_match_score = 0.0f;
        for(int i = 0; i < w_k * h_k; i++) {
            ker[i] -= avg_k; // Zero-mean کردن
            // محاسبه سقف امتیاز: اگر یک پیکسل ۲۵۵ در وزن مثبت ضرب شود
            if (ker[i] > 0) {
                perfect_match_score += ker[i] * 255.0f;
            }
        }

        // ۳. اجرای کانولوشن (با نسخه اسمبلی یا C)
        float* out_c = calloc(w*h, sizeof(float));
        conv2d_c(in, out_c, ker, w, h, w_k);
        writePGM("output_c.pgm", out_c, w, h);


        // ۱. پیدا کردن ماکزیمم و میانگین خروجی
        // ۱. محاسبه انرژی خودِ الگو (Template Energy)
        float energy_k = 0;
        for(int i = 0; i < w_k * h_k; i++) {
            energy_k += ker[i] * ker[i];
        }

        float max_ncc = -1.0f;
        int best_x = 0, best_y = 0;

        for (int y = 0; y < h - h_k; y++) {
            for (int x = 0; x < w - w_k; x++) {
                // ۲. محاسبه انرژی تکه‌ای از تصویر که زیر الگو قرار دارد (Window Energy)
                float energy_win = 0;
                for (int ky = 0; ky < h_k; ky++) {
                    for (int kx = 0; kx < w_k; kx++) {
                        float pixel = in[(y + ky) * w + (x + kx)];
                        energy_win += pixel * pixel;
                    }
                }

                // ۳. مقدار خروجی کانولوشن در این نقطه
                float val = out_c[y * w + x];

                // ۴. فرمول NCC: همبستگی تقسیم بر جذر ضرب انرژی‌ها
                float denom = sqrtf(energy_k * energy_win);
                float ncc = (denom > 0) ? (val / denom) : 0;

                if (ncc > max_ncc) {
                    max_ncc = ncc;
                    best_x = x;
                    best_y = y;
                }
            }
        }

        // ۵. نمایش نتیجه نهایی
        float confidence = max_ncc * 100.0f;
        if (confidence < 0) confidence = 0;
        if (confidence > 100) confidence = 100;

        printf("Max NCC Score: %.4f\n", max_ncc);
        printf("Result: %s (Confidence: %.2f%%) at [%d, %d]\n", 
            (confidence > 85.0f) ? "FOUND" : "NOT FOUND", confidence, best_x, best_y);
        // ۱. پیدا کردن مقدار ماکزیمم واقعی که اسمبلی/C تولید کرده
        float absolute_max = -1e20f; 
        for (int i = 0; i < w * h; i++) {
            if (out_c[i] > absolute_max) absolute_max = out_c[i];
        }

        // ۲. تمیز کردن تصویر: فقط قله‌ها سفید، بقیه سیاه مطلق
        for (int i = 0; i < w * h; i++) {
            // اگر مقدار پیکسل بیشتر از ۹۵٪ِ ماکزیمم بود (یعنی خودِ الگوست)
            if (out_c[i] >= absolute_max * 0.95f && absolute_max > 0) {
                out_c[i] = 255.0f; // سفید درخشان
            } else {
                out_c[i] = 0.0f;   // سیاه مطلق
            }
        }

        // حالا تصویر را ذخیره کن

        writePGM("out_centerOfPattern.pgm", out_c, w, h);
        return 0;
    }

    // اگر پارامتر های لازم ورودی را نداشت -> خروج از برنامه 
    if (argc < 3) {
    printf("Usage: %s <Blur|Sharpen|Edge_Detection>  <name of picture(.pmg)>\n", argv[0]);
    return 1;
    }

    int w, h;    
    // ساخت ادرس تصویر ورودی
    char base[256] = "inputs/";
    strcat(base, argv[2]);     
    char* input = base;

    //خواندن تصویر
    float* in = readPGM(input, &w, &h);

    //درست کردن ارایه خروحی 
    float* out_c = calloc(w*h, sizeof(float));
    float* out_asm = calloc(w*h , sizeof(float));

    //طول و عرض کرنل
    int k = 3;
    double err = 0;
    double t1;
    double t2;
    double t3;
    double t4;
    int count = 0;
        // Edge_Detection 
        float ker_Edge[9] = {
            -1,-1,-1,
            -1,8,-1,
            -1,-1,-1
        };

        // Blur 
        float ker_Blur[9] = {
            1.00f/9.00f,1.00f/9.00f,1.00f/9.00f,
            1.00f/9.00f,1.00f/9.00f,1.00f/9.00f,
            1.00f/9.00f,1.00f/9.00f,1.00f/9.00f
        };

        // Sharpen
        float ker_sharpen[9] = {
        0, -1,  0,
        -1,  5, -1,
        0, -1,  0
        };

        //انتخاب کرنل با توجه به فیلتر انتخابی
        float ker[9];
        //Blur
        if (!strcmp(argv[1],"Blur")) {
            memcpy(ker,ker_Blur,sizeof(ker));
        }
        // Sharpen
        else if (!strcmp(argv[1],"Sharpen")) {
            memcpy(ker,ker_sharpen,sizeof(ker));
        }
        //Edge_Detection
        else if (!strcmp(argv[1],"Edge_Detection")){
            memcpy(ker,ker_Edge,sizeof(ker));
        }

        // اجرای convolution در c
        t1 = now(); //ثبت تایم شروع 
        conv2d_c(in, out_c, ker, w, h, k); //صدا زدن تابع کانولوشن سی
        t2 = now(); // ثبت تایم پایان
        // نوشتن تصویر
        writePGM("output_c.pgm", out_c, w, h);


        //zero paddings
        int pad = k/2;
        int h2 = h + 2*pad;
        // تعداد سطر ها بر ۸ بخش پذیر باشد تا در هشت تا هشت تا خوندن اعداد به مشکل نخوریم
        int w2 = w + 2*pad;
        int w_pad = ((w2 + 7) / 8) * 8;
        w_pad += 2*pad;

        float* in_pad  = calloc(w_pad*h2, sizeof(float));
        float* out_pad = calloc(w_pad*h2, sizeof(float));
        //padding
        for(int y=0; y<h; y++){
            memcpy(in_pad + (y+pad)*w_pad + pad,in + y*w,w*sizeof(float)); //(تعداد عدد خوانده شده,ادرس مبدا,ادرس مقصد)
        }



        //اجرای convolution در asm
        t3 = now(); //ثبت تایم شروع 
        conv2d_asm(in_pad, out_pad, ker, w_pad, h2, k); //صدا زدن تابع کانولوشن اسمبلی با دستورات برداری و سرعت بهینه
        t4 = now(); // ثبت تایم پایان

        // استخراج تصویر اصلی از تصویر پدینگ شده
        float* out_final_asm = malloc(w*h*sizeof(float));
        for(int y=0; y<h; y++){
            memcpy(out_final_asm + y*w,
            out_pad + (y+pad)*w_pad + pad,
            w*sizeof(float));
        }
        // نوشتن تصویر
        writePGM("output_asm.pgm", out_final_asm, w, h);
        
        //محاسبه خطا اسمبلی و سی نسبت به هم
        int cnt = 0;
        for(int y=0; y<h; y++){
            for(int x=0; x<w; x++){
                int i = y*w + x;
                err += fabs(out_c[i] - out_final_asm[i]);
                cnt++;
            }
        }

        err /= cnt;
    //گزارش نتایج
    printf("C time = %f\n", t2-t1);
    printf("ASM time = %f\n", t4-t3);
    printf("speedup : %f\n",(t2-t1) / (t4-t3));
    printf("error : %f\n",err);


    //ازاد سازی حافظه 
    free(in);
    free(out_c);
    free(out_asm);

    return 0;
}
