# کامپایلر C
CC=gcc

# اسمبلر NASM
ASM=nasm

# فلگ‌های کامپایل:
# -O3 : بیشترین سطح بهینه‌سازی
# -mavx : فعال‌سازی دستورات AVX
CFLAGS=-O3 -mavx


# تارگت پیش‌فرض <- ساخت فایل اجرایی main
all: main


# تبدیل فایل اسمبلی به آبجکت
conv.o: conv.asm
	$(ASM) -f elf64 conv.asm -o conv.o


# کامپایل و لینک کل پروژه
# -lm برای توابع ریاضی مثل fabs,...
main: main.c conv.c timer.c conv.o
	$(CC) $(CFLAGS) main.c conv.c timer.c conv.o -lm -o main


# پاک کردن فایل‌های ساخته شده
clean:
	rm -f main *.o output_c.pgm output_asm.pgm
