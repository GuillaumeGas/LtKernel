nasm -f bin -o bootsect bootsect.asm
nasm -f elf -o kernel.o kernel.asm
gcc -m32 -c screen.c
ld -m elf_i386 --oformat binary -Ttext 10000 kernel.o screen.o -o kernel
cat bootsect kernel /dev/zero | dd of=floppyA bs=512 count=2880
bochs 'boot:a' 'floppya: 1_44=floppyA, status=inserted'