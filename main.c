#include "conv.h" //شناساندن توابع conv به main
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

double now(); //پروتوتایپ تابع مربوط به زمان در فایل timer.c


float* readPGM(const char* name, int* w, int* h)
{
    FILE* f = fopen(name, "rb");
    if (!f) { printf("Cannot open input\n"); exit(1); }

    char m[3];

    if (fscanf(f, "%2s", m) != 1) exit(1);
    if (strcmp(m,"P5") != 0) exit(1);

    if (fscanf(f, "%d %d", w, h) != 2) exit(1);

    int max;
    if (fscanf(f, "%d", &max) != 1) exit(1);

    fgetc(f);

    unsigned char* tmp = malloc((*w)*(*h));

    size_t n = fread(tmp, 1, (*w)*(*h), f);
    if (n != (*w)*(*h)) exit(1);

    fclose(f);

    float* img = malloc(sizeof(float)*(*w)*(*h));

    for(int i=0;i<(*w)*(*h);i++)
        img[i] = (float)tmp[i];

    free(tmp);

    return img;
}



void writePGM(const char* name, float* img, int w, int h)
{
    FILE* f = fopen(name, "wb");

    fprintf(f,"P5\n%d %d\n255\n", w, h);

    for(int i=0;i<w*h;i++)
    {
        float v = img[i];

        if(v < 0) v = 0;
        if(v > 255) v = 255;

        unsigned char c = (unsigned char)(v);
        fwrite(&c,1,1,f);
    }

    fclose(f);
}


int main(int argc, char *argv[])
{
    if (argc < 3) {
    printf("Usage: %s <Blur|Sharpen|Edge_Detection>  <name of picture(.pmg)>\n", argv[0]);
    return 1;
    }

    int w, h,T;
    T = 60;
    char base[256] = "inputs/";
    strcat(base, argv[2]);     
    char* input = base;

    float* in = readPGM(input, &w, &h);

    float* out_c = calloc(w*h, sizeof(float));
    float* out_asm = calloc(w*h , sizeof(float));

    int k = 3;


    // float* out_c2 = calloc(w*h, sizeof(float));
    // float* out_asm2 = calloc(w*h , sizeof(float));
    // float sobelX[9]= {
    //     -1, 0, 1,
    //     -2, 0, 2,
    //     -1, 0, 1

    // };
    // float sobelY[9]= {
    //     -1, -2, -1,
    //     0,  0,  0,
    //     1,  2, 1
    // };

    // // double t5 = now();
    // // conv2d_c(in, out_c, sobelX, w, h, k);
    // // double t6 = now();

    // double t7 = now();
    // conv2d_asm(in, out_asm, sobelX, w, h, k);
    // double t8 = now();

    // // double t9 = now();
    // // conv2d_c(in, out_c2, sobelY, w, h, k);
    // // double t10 = now();

    // double t11 = now();
    // conv2d_asm(in, out_asm2, sobelY, w, h, k);
    // double t12 = now();

    // float* mag = calloc(w*h,sizeof(float));

    // for (int i =0 ;i<w*h ;i++) {
    //     mag[i]= out_asm[i]*out_asm[i] + out_asm2[i] * out_asm2[i];
    // }
    // int count = 0;
    // for (int i = 0 ; i < h * w ; i++) {
    //     if (mag[i] > T * T) {
    //         count++;
    //     }
    // }
    // if(count > w*h*0.05)
    //     printf("object detected\n");
    // else
    //     printf("no object\n");





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
    double t1 = now();
    conv2d_c(in, out_c, ker, w, h, k);
    double t2 = now();
    writePGM("output_c.pgm", out_c, w, h);


    //zero paddings
    int pad = k/2;
    int h2 = h + 2*pad;
    int w2 = w + 2*pad;
    int w_pad = ((w2 + 7) / 8) * 8;
    w_pad += 2;

    float* in_pad  = calloc(w_pad*h2, sizeof(float));
    float* out_pad = calloc(w_pad*h2, sizeof(float));

    for(int y=0; y<h; y++){
        memcpy(in_pad + (y+pad)*w_pad + pad,in + y*w,w*sizeof(float)); //(تعداد عدد خوانده شده,ادرس مبدا,ادرس مقصد)
    }



    //اجرای convolution در asm
    double t3 = now();
    conv2d_asm(in_pad, out_pad, ker, w_pad, h2, k);
    double t4 = now();


    float* out_final_asm = malloc(w*h*sizeof(float));
    for(int y=0; y<h; y++){
        memcpy(out_final_asm + y*w,
        out_pad + (y+pad)*w_pad + pad,
        w*sizeof(float));
    }
    
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
