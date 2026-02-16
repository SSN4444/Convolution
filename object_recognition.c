#include "conv.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

double now();
float* readPGM(const char*, int*, int*);
void writePGM(const char*, float*, int, int);


// تشخیص شی با Sobel + threshold
static int detect_object(float* img, int w, int h, float T)
{
    int count = 0;

    for(int i=0;i<w*h;i++)
        if(img[i] > T) count++;

    // اگر بیش از ۵٪ تصویر لبه داشت → شی وجود دارد
    return (count > 0.05f*w*h);
}



void objectRecognition(const char* folder, int N)
{
    int k = 3;
    float T = 60.0f;

    // کرنل Sobel
    float sobelX[9] = {
        -1,0,1,
        -2,0,2,
        -1,0,1
    };

    float sobelY[9] = {
        -1,-2,-1,
         0, 0, 0,
         1, 2, 1
    };


    double time_c = 0;
    double time_asm = 0;

    int found_c = 0;
    int found_asm = 0;


    //  loop روی تصاویر 
    for(int id=1; id<=N; id++)
    {
        char name[256];
        sprintf(name, "%s/input%d.pgm", folder, id);

        int w,h;

        float* in = readPGM(name, &w, &h); //خواندن تصویر

        //ارایه گرادیان ها 
        float* gx_c  = calloc(w*h,sizeof(float)); 
        float* gy_c  = calloc(w*h,sizeof(float));
        float* mag_c = calloc(w*h,sizeof(float));

        float* gx_a  = calloc(w*h,sizeof(float));
        float* gy_a  = calloc(w*h,sizeof(float));
        float* mag_a = calloc(w*h,sizeof(float));


        //  C 
        double t1 = now();

        conv2d_c(in, gx_c, sobelX, w, h, k);
        conv2d_c(in, gy_c, sobelY, w, h, k);
        //محاسبه اندازه بردار گرادیان
        for(int i=0;i<w*h;i++)
            mag_c[i] = sqrtf(gx_c[i]*gx_c[i] + gy_c[i]*gy_c[i]);

        double t2 = now();
        time_c += (t2-t1);
        //ایا تصویر شی دارد؟
        if(detect_object(mag_c,w,h,T))
            found_c++;


        //  ASM 

        //zero paddings
        int pad = k/2;
        int h2 = h + 2*pad;
        // تعداد سطر ها بر ۸ بخش پذیر باشد تا در هشت تا هشت تا خوندن اعداد به مشکل نخوریم
        int w2 = w + 2*pad;
        int w_pad = ((w2 + 7) / 8) * 8;
        w_pad += 2;

        float* in_pad  = calloc(w_pad*h2, sizeof(float));
        float* out_pad_gx = calloc(w_pad*h2, sizeof(float));
        float* out_pad_gy = calloc(w_pad*h2, sizeof(float));
        //padding
        for(int y=0; y<h; y++){
            memcpy(in_pad + (y+pad)*w_pad + pad,in + y*w,w*sizeof(float)); //(تعداد عدد خوانده شده,ادرس مبدا,ادرس مقصد)
        }


        //اجرای convolution در asm
        double t3 = now();
        
        conv2d_asm(in_pad, out_pad_gx, sobelX, w_pad, h2, k);
        conv2d_asm(in_pad, out_pad_gy, sobelY, w_pad, h2, k);

        // استخراج تصویر اصلی از تصویر پدینگ شده
        for(int y=0; y<h; y++){
            memcpy(gx_a + y*w,
            out_pad_gx + (y+pad)*w_pad + pad,
            w*sizeof(float));
        }
        for(int y=0; y<h; y++){
            memcpy(gy_a + y*w,
            out_pad_gy + (y+pad)*w_pad + pad,
            w*sizeof(float));
        }
        //محاسبه اندازه بردار گرادیان
        for(int i=0;i<w*h;i++)
            mag_a[i] = sqrtf(gx_a[i]*gx_a[i] + gy_a[i]*gy_a[i]);

        double t4 = now();
        time_asm += (t4-t3);

        // ایا تصویر شی دارد؟
        if(detect_object(mag_a,w,h,T))
            found_asm++;

        //ازاد سازی حافظه
        free(in);
        free(gx_c); free(gy_c); free(mag_c);
        free(gx_a); free(gy_a); free(mag_a);
        free(in_pad); free(out_pad_gx); free(out_pad_gy);

    }


    //  گزارش 

    printf("REPORT \n");

    printf("C   total time  : %f sec\n", time_c);
    printf("ASM total time  : %f sec\n", time_asm);
    printf("Speedup         : %.2fx\n", time_c/time_asm);

    printf("Objects (C)     : %d / %d\n", found_c, N);
    printf("Objects (ASM)   : %d / %d\n", found_asm, N);
}
