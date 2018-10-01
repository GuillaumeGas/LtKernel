OBJ=kernel
CC=i686-elf-gcc.exe -ffreestanding -nostdlib -Wall -m32 -g -fno-stack-protector -fno-pie -I ../
ASM=nasm -f elf
LD=i686-elf-ld

ROOT=kernel.o logger.o scheduler.o
DEBUG=debug.o
DRIVERS=screen.o serial.o
INIT=gdt.o gdt_loader.o idt.o idt_loader.o int.o isr.o memory.o pic.o vmm_loader.o
LIB=kmalloc.o list.o stdio.o stdlib.o
USER=process_manager.o process_starter.o

all: $(OBJ) 

clean:
	rm -f $(OBJ) *.o

kernel: $(ROOT) $(DEBUG) $(DRIVERS) $(INIT) $(LIB) $(USER)
	$(LD) -m elf_i386 --oformat binary -Ttext 1000 $^ -o $@

# GENERAL STUFF
.o: .c 
	$(CC) -c $^

# ROOT DIRECTORY
logger.o: logger.c
	$(CC) -c $^

scheduler.o: scheduler.c
	$(CC) -c $^

# DEBUG DIRECTORY
debug.o: debug/debug.c
	$(CC) -c $^

# DRIVERS DIRECTORY
screen.o: drivers/screen.c
	$(CC) -c $^

serial.o: drivers/serial.c
	$(CC) -c $^

# INIT DIRECTORY
gdt.o: init/gdt.c
	$(CC) -c $^

gdt_loader.o: init/gdt_loader.asm
	$(ASM) -o $@ $^

idt.o: init/idt.c
	$(CC) -c $^

idt_loader.o: init/idt_loader.asm
	$(ASM) -o $@ $^

int.o: init/int.asm
	$(ASM) -o $@ $^

isr.o: init/isr.c
	$(CC) -c $^

memory.o: init/memory.c
	$(CC) -c $^

pic.o: init/pic.asm
	$(ASM) -o $@ $^

vmm_loader.o: init/vmm_loader.asm
	$(ASM) -o $@ $^

# LIB DIRECTORY
kmalloc.o: lib/kmalloc.c
	$(CC) -c $^

list.o: lib/list.c
	$(CC) -c $^

stdio.o: lib/stdio.c
	$(CC) -c $^

stdlib.o: lib/stdlib.c
	$(CC) -c $^

# USER DIRECTORY
process_manager.o: user/process_manager.c
	$(CC) -c $^

process_starter.o: user/process_starter.asm
	$(ASM) -o $@ $^