OBJ=kernel
CC=gcc -ffreestanding -nostdlib -Wall -m32 -g -fno-stack-protector -fno-pie -I ../
ASM=nasm -f elf32
LD=ld

BOOT=../boot/bootsector.o
ROOT=kernel.o logger.o scheduler.o
DEBUG=debug.o
DRIVERS=screen.o serial.o clock.o keyboard.o ata.o
INIT=gdt.o gdt_loader.o idt.o idt_loader.o int.o isr.o vmm.o pic.o vmm_loader.o heap.o
LIB=kmalloc.o list.o stdio.o stdlib.o panic.o
USER=process_manager.o process_starter.o syscalls.o user_tests.o

all: $(OBJ) 

clean:
	rm -f $(OBJ) *.o

kernel: $(BOOT) $(ROOT) $(DEBUG) $(DRIVERS) $(INIT) $(LIB) $(USER)
	$(LD) -m elf_i386 -nostdlib -T linker.ld $^ -o $@

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

clock.o: drivers/clock.c
	$(CC) -c $^

keyboard.o: drivers/keyboard.c
	$(CC) -c $^

ata.o: drivers/ata.c
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

vmm.o: init/vmm.c
	$(CC) -c $^

pic.o: init/pic.asm
	$(ASM) -o $@ $^

vmm_loader.o: init/vmm_loader.asm
	$(ASM) -o $@ $^

heap.o: init/heap.c
	$(CC) -c $^

# LIB DIRECTORY
kmalloc.o: lib/kmalloc.c
	$(CC) -c $^

list.o: lib/list.c
	$(CC) -c $^

stdio.o: lib/stdio.c
	$(CC) -c $^

stdlib.o: lib/stdlib.c
	$(CC) -c $^

panic.o: lib/panic.c
	$(CC) -c $^

# USER DIRECTORY
process_manager.o: user/process_manager.c
	$(CC) -c $^

process_starter.o: user/process_starter.asm
	$(ASM) -o $@ $^

syscalls.o: user/syscalls.c
	$(CC) -c $^

user_tests.o: user/user_tests.c
	$(CC) -c $^
