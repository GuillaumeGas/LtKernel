OBJ=kernel
CC=gcc -Wall -m32

all: $(OBJ) 

kernel: kernel.o screen.o gdt.o memory.o
	ld -m elf_i386 --oformat binary -Ttext 10000 $^ -o $@

.o: .c 
	$(CC) -c $^

clean:
	rm -f $(OBJ) *.o
