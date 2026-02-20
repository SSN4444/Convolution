#pragma once // جلوگیری از چندبار include شدن فایل هدر (جلوگیری از خطای تکرار تعریف)
// اجرای تشخیص  الگو روی چند تصویر
// folder : پوشه تصاویر (مثلاً inputs)
// count  : تعداد تصاویر
// kernel : ادرس الگو kernel
void patternRecognition(const char* folder, int count,const char* kernel);