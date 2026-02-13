#include <time.h>

double now(){
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);  // گرفتن تایم از MONOTONIC
    return ts.tv_sec + ts.tv_nsec*1e-9; // برگرداندن تایم به واحد ثانیه
}
