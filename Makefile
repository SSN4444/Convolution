CC=gcc
ASM=nasm

CFLAGS=-O3 -mavx

all: main

conv.o: conv.asm
	$(ASM) -f elf64 conv.asm -o conv.o

main: main.c conv.c timer.c conv.o
	$(CC) $(CFLAGS) main.c conv.c timer.c conv.o -lm -o main

clean:
	rm -f main *.o output_c.pgm output_asm.pgm



