OBJ=floppyA 

all: $(OBJ) 

floppyA: bootsect kern
	cat boot/bootsect kernel/kernel /dev/zero | dd of=floppyA bs=512 count=2880

bootsect: 
	make -C boot

kern: 
	make -C kernel

clean:
	rm -f $(OBJ) *.o
	make -C boot clean
	make -C kernel clean
