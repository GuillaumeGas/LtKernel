OBJ=bootsectStageOne bootsectStageTwo

all: $(OBJ) 

bootsectStageOne: stageOne/bootsect.asm
	nasm -f bin -o $@ $^

bootsectStageTwo: stageTwo/stageTwo.asm
	nasm -f bin -o $@ $^

clean:
	rm -f bootsectStageOne
	rm -f bootsectStageTwo