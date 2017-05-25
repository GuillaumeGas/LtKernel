[BITS 32]

EXTERN print, clear, setColor
GLOBAL _start

%define BLACK 0x0
%define WHITE 0x0F
%define RED   0x4E
	
_start:
	call clear

	mov eax, WHITE
	push eax
	call setColor
	
	mov eax, titleMsg
	push eax
	call print

	mov eax, RED
	push eax
	call setColor
	
	mov eax, helloMsg
	push eax
	call print
end:
    jmp end

titleMsg db "< ## LtKernel ## >", 10, 0
helloMsg db "Hello from LtKernel !", 10, 0