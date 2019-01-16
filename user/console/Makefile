OBJ=main.out
INC_DIR=../lib
CC=gcc -m32 -ffreestanding -nostdlib -Wall -fno-stack-protector -fno-pie -I$(INC_DIR)
ASM=nasm -f elf32
LD=ld -Ttext=40000000 -m elf_i386 --entry=main

all: $(OBJ)

clean:
	rm -f $(OBJ) *.o

main.out: main.o stdio.o stdlib.o syscalls.o
	$(LD) $^ -o $@
	e2cp $@ ../disk.img:/

main.o: main.c 
	$(CC) -c $^

stdio.o: ../lib/stdio.c 
	$(CC) -c $^

stdlib.o: ../lib/stdlib.c 
	$(CC) -c $^

syscalls.o: ../lib/syscalls.asm
	$(ASM) -o $@ $^
