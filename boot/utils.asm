[BITS 16]
[ORG 0x0]

;;; Print function
;;; Param : string, its address must be in si
print:
	push bx
beginPrint:	
	lodsb			; mov ah, byte at si
	cmp al, 0
	jz endPrint
	mov ah, 0x0E
	mov bx, 0x07
	int 0x10
	jmp beginPrint
endPrint:
	pop bx
	ret