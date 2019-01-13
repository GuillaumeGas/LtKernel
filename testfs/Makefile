OBJ=main.out
CC=gcc -m32
LD=ld -Ttext=40000000 -m elf_i386 --entry=main

all: $(OBJ) 

clean:
	rm -f $(OBJ) *.o

main.out: main.o
	$(LD) $^ -o $@
	e2cp $@ disk.img:/

# GENERAL STUFF
.o: .c 
	$(CC) -c $^