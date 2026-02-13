#pragma once

void conv2d_c(float* in, float* out, float* kernel,
              int w, int h, int k);

void conv2d_asm(float* in, float* out, float* kernel,
                int w, int h, int k);
