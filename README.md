# 🚀 Fast 2D Convolution in C & x86-64 Assembly (AVX Optimized)

📌 Computer Architecture / Low-Level Programming Project  
📎 Repo: https://github.com/SSN4444/Convolution.git

---

## 🎯 Project Goal

هدف این پروژه پیاده‌سازی و مقایسه‌ی **کانولوشن دوبعدی تصویر (2D Convolution)** به دو روش است:

1️⃣ پیاده‌سازی ساده با C (Baseline)  
2️⃣ پیاده‌سازی بسیار سریع با Assembly + AVX SIMD  

سپس:
- مقایسه زمان اجرا (Benchmark)
- محاسبه Speedup
- Zero Padding
- تشخیص لبه با Sobel
- تشخیص شی (Object Recognition)
- اجرای خودکار روی صدها تصویر

---

## ⚙️ Features

✅ 2D Convolution (C version)  
✅ 2D Convolution (Assembly + AVX version)  
✅ SIMD Vectorization (پردازش 8 پیکسل همزمان)  
✅ Zero Padding  
✅ Sobel Edge Detection  
✅ Threshold-based Object Detection  
✅ Batch Processing (100–400 images)  
✅ Speedup Measurement  
✅ PGM Image Support  

---

## 🧠 Algorithms

### 🔹 Convolution
اعمال کرنل 3×3 روی تصویر:

```
sum += pixel * weight
```

### 🔹 Zero Padding
افزودن صفر در اطراف تصویر برای جلوگیری از خطای مرزی

### 🔹 Sobel Operator
محاسبه گرادیان‌های افقی و عمودی:

```
Gx = SobelX * image
Gy = SobelY * image
Magnitude = sqrt(Gx² + Gy²)
```

### 🔹 Object Detection
اگر بیش از ۵٪ پیکسل‌ها گرادیان بزرگ داشته باشند:
```
→ object detected
```

---

## 📂 Project Structure

```
.
├── main.c                  # فایل اصلی برنامه
├── conv.c                  # نسخه C کانولوشن
├── conv.asm                # نسخه Assembly + AVX
├── conv.h                  # header
├── timer.c                 # اندازه‌گیری زمان
├── object_recognition.c    # پردازش 100/400 تصویر
├── inputs/                 # تصاویر ورودی
└── Makefile
```

---

## 🛠 Build

### با Makefile
```bash
make
```

### یا دستی
```bash
nasm -f elf64 conv.asm -o conv.o
gcc -O3 -mavx main.c conv.c timer.c object_recognition.c conv.o -lm -o main
```

---

## ▶️ Run

### اجرای یک تصویر
```bash
./main Blur input1.pgm
```

### اجرای دسته‌ای روی چند تصویر
```bash
./main object_recognition
```

---

## 📊 Sample Output

```
C time   = 0.007040
ASM time = 0.000933
speedup  = 7.54x
error    = 0.000000
```

---

## 🚀 Optimization Techniques (Assembly)

- AVX (ymm registers)
- پردازش 8 float همزمان
- vbroadcastss برای reuse وزن‌ها
- Memory alignment
- Scalar fallback برای لبه‌ها
- Zero padding برای حذف branch
- vzeroupper برای جلوگیری از penalty

---

## 🖼 Image Format

فرمت ورودی:
```
PGM (P5)
Grayscale 0–255
```

---

## 📈 Performance Summary

| Version | Speed |
|--------|--------|
| C | 1x (baseline) |
| ASM + AVX | ~7–10x faster |

---

## 🎓 Learning Outcomes

با این پروژه یاد می‌گیرید:

- SIMD / AVX Programming
- x86-64 Assembly
- Memory alignment
- Image Processing basics
- Performance optimization
- Benchmarking
- Low-level debugging

---

## 👨‍💻 Author

Student Project – Low-Level Optimization & SIMD Programming

---

## 📜 License

Educational / Academic Use Only
