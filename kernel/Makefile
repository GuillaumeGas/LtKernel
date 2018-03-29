OBJ=kernel
CC=gcc -Wall -m32 -fno-pie

all: $(OBJ) 

kernel: kernel.o screen.o gdt.o memory.o idt.o
	ld -m elf_i386 --oformat binary --ignore-unresolved-symbol _GLOBAL_OFFSET_TABLE_ -Ttext 10000 $^ -o $@

.o: .c 
	$(CC) -c $^

clean:
	rm -f $(OBJ) *.o
