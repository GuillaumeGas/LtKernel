OBJ=kernel
CC=gcc -Wall -m32 -g

all: $(OBJ) 

kernel: kernel.o screen.o memory.o idt.o int.o isr.o gdt.o pic.o
	ld -m elf_i386 --oformat binary --ignore-unresolved-symbol _GLOBAL_OFFSET_TABLE_ -Ttext 1000 $^ -o $@

int.o: int.asm
	nasm -f elf -o $@ $^

gdt.o: gdt.asm
	nasm -f elf -o $@ $^

idt.o: idt.asm
	nasm -f elf -o $@ $^

pic.o: pic.asm
	nasm -f elf -o $@ $^

.o: .c 
	$(CC) -c $^

clean:
	rm -f $(OBJ) *.o
