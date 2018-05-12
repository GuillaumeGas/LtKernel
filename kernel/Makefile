OBJ=kernel
CC=gcc -Wall -m32 -g -fno-stack-protector -fno-pie

all: $(OBJ) 

kernel: kernel.o idt.o idt_loader.o int.o isr.o gdt.o pic.o gdt_loader.o screen.o serial.o memory.o stdio.o task_switch.o
	ld -m elf_i386 --oformat binary -Ttext 1000 $^ -o $@

memory.o: lib/memory.c
	$(CC) -c $^

stdio.o: lib/stdio.c
	$(CC) -c $^

int.o: init/int.asm
	nasm -f elf -o $@ $^

gdt_loader.o: init/gdt_loader.asm
	nasm -f elf -o $@ $^

idt.o: init/idt.c
	$(CC) -c $^

idt_loader.o: init/idt_loader.asm
	nasm -f elf -o $@ $^

isr.o: init/isr.c
	$(CC) -c $^

gdt.o: init/gdt.c
	$(CC) -c $^

pic.o: init/pic.asm
	nasm -f elf -o $@ $^

screen.o: drivers/screen.c
	$(CC) -c $^

serial.o: drivers/serial.c
	$(CC) -c $^

task_switch.o: task_switch.asm
	nasm -f elf -o $@ $^

.o: .c 
	$(CC) -c $^

clean:
	rm -f $(OBJ) *.o
