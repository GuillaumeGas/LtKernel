OBJ=kernel
CC=gcc -Wall -m32 -g

all: $(OBJ) 

kernel: kernel.o screen.o memory.o idt.o int.o isr.o agdt.o aidt.o
	ld -m elf_i386 --oformat binary --ignore-unresolved-symbol _GLOBAL_OFFSET_TABLE_ -Ttext 1000 $^ -o $@

int.o: int.asm
	nasm -f elf -o $@ $^

agdt.o: agdt.asm
	nasm -f elf -o $@ $^

aidt.o: aidt.asm
	nasm -f elf -o $@ $^

.o: .c 
	$(CC) -c $^

clean:
	rm -f $(OBJ) *.o
