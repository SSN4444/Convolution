#include "conv.h" // اعلان پروتوتایپ توابع convolution برای استفاده در فایل‌های دیگر
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

double now(); //پروتوتایپ تابع مربوط به زمان در فایل timer.c


// خواندن تصویر PGM و تبدیل آن به آرایه float
float* readPGM(const char* name, int* w, int* h)
{
    FILE* f = fopen(name, "rb");   // باز کردن فایل باینری
    if (!f) { printf("Cannot open input\n"); exit(1); }

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
    // اگر پارامتر های لازم ورودی را نداشت -> خروج از برنامه 
    if (argc < 3) {
    printf("Usage: %s <Blur|Sharpen|Edge_Detection>  <name of picture(.pmg)>\n", argv[0]);
    return 1;
    }

    int w, h,T;
    T = 60;
    
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

    // Edge_Detection 
    float ker_Edge[9] = {
        -1,-1,-1,
        -1, 8,-1,
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
    double t1 = now(); //ثبت تایم شروع 
    conv2d_c(in, out_c, ker, w, h, k); //صدا زدن تابع کانولوشن سی
    double t2 = now(); // ثبت تایم پایان
    // نوشتن تصویر
    writePGM("output_c.pgm", out_c, w, h);


    //zero paddings
    int pad = k/2;
    int h2 = h + 2*pad;
    // تعداد سطر ها بر ۸ بخش پذیر باشد تا در هشت تا هشت تا خوندن اعداد به مشکل نخوریم
    int w2 = w + 2*pad;
    int w_pad = ((w2 + 7) / 8) * 8;
    w_pad += 2;

    float* in_pad  = calloc(w_pad*h2, sizeof(float));
    float* out_pad = calloc(w_pad*h2, sizeof(float));
    // استخراج تصویر اصلی از تصویر پدینگ شده
    for(int y=0; y<h; y++){
        memcpy(in_pad + (y+pad)*w_pad + pad,in + y*w,w*sizeof(float)); //(تعداد عدد خوانده شده,ادرس مبدا,ادرس مقصد)
    }



    //اجرای convolution در asm
    double t3 = now(); //ثبت تایم شروع 
    conv2d_asm(in_pad, out_pad, ker, w_pad, h2, k); //صدا زدن تابع کانولوشن اسمبلی با دستورات برداری و سرعت بهینه
    double t4 = now(); // ثبت تایم پایان


    float* out_final_asm = malloc(w*h*sizeof(float));
    for(int y=0; y<h; y++){
        memcpy(out_final_asm + y*w,
        out_pad + (y+pad)*w_pad + pad,
        w*sizeof(float));
    }
    // نوشتن تصویر
    writePGM("output_asm.pgm", out_final_asm, w, h);

    
    //محاسبه خطا اسمبلی و سی نسبت به هم
    double err = 0;
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
