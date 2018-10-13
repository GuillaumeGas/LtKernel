OBJ=img
DD="C:\MinGW\msys\1.0\bin\dd.exe"

all: $(OBJ) 

img: bootsect kern
	cp kernel/kernel iso/boot/ltkernel.img

bootsect: 
	mingw32-make -C boot

kern: 
	mingw32-make -C kernel

clean:
	rm -f $(OBJ) kernel.bin iso/boot/ltkernel.img *.o
	mingw32-make -C boot clean
	mingw32-make -C kernel clean
