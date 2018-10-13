OBJ=bootsector.o

all: $(OBJ) 

bootsector.o: bootsector.asm
	nasm -f elf -o $@ $^

clean:
	rm -f bootsector.o
