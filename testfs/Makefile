OBJ=main.out
CC=gcc -m32 -ffreestanding -nostdlib -Wall -fno-stack-protector -fno-pie
ASM=nasm -f elf32
LD=ld -Ttext=40000000 -m elf_i386 --entry=main

all: $(OBJ) 

clean:
	rm -f $(OBJ) *.o

main.out: main.o syscalls.o
	$(LD) $^ -o $@
	e2cp $@ disk.img:/

main.o: main.c 
	$(CC) -c $^

syscalls.o: syscalls.asm
	$(ASM) -o $@ $^
