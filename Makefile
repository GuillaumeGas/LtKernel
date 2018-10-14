OBJ=img

all: $(OBJ) 

img: bootsect kern
	cp kernel/kernel iso/boot/ltkernel.img
	grub-mkrescue -o ltkernel.iso iso

bootsect: 
	make -C boot

kern: 
	make -C kernel

clean:
	rm -f $(OBJ) kernel.bin iso/boot/ltkernel.img *.o ltkernel.iso
	make -C boot clean
	make -C kernel clean
