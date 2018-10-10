OBJ=img
DD="C:\MinGW\msys\1.0\bin\dd.exe"

all: $(OBJ) 

img: bootsect kern
	cat boot/bootsectStageOne boot/bootsectStageTwo kernel/kernel >> kernel.bin | $(DD) if=kernel.bin of=ltkernel.img bs=512 count=2880

bootsect: 
	mingw32-make -C boot

kern: 
	mingw32-make -C kernel

clean:
	rm -f $(OBJ) kernel.bin ltkernel.img *.o
	mingw32-make -C boot clean
	mingw32-make -C kernel clean
