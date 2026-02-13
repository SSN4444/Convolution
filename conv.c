#include "conv.h"

void conv2d_c(float* in, float* out, float* ker,
              int w, int h, int k)
{
    int pad = k / 2;

    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            float sum = 0.0f;

            for (int ky = 0; ky < k; ky++)
            {
                for (int kx = 0; kx < k; kx++)
                {
                    int ix = x + kx - pad;
                    int iy = y + ky - pad;

                    if (ix >= 0 && ix < w && iy >= 0 && iy < h)
                    {
                        float pixel = in[iy*w + ix];

                        float weight = ker[(k-1-ky)*k + (k-1-kx)];

                        sum += pixel * weight;
                    }
                }
            }

            out[y*w + x] = sum;
        }
    }
}