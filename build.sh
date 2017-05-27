nasm -f bin -o bootsect_bin boot/bootsect.asm
gcc -m32 -c drivers/screen.c kernel/kernel.c
ld -m elf_i386 --oformat binary -Ttext 10000 kernel.o screen.o -o kernel_bin
cat bootsect_bin kernel_bin /dev/zero | dd of=floppyA bs=512 count=2880
bochs 'boot:a' 'floppya: 1_44=floppyA, status=inserted'