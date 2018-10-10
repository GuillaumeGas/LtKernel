[BITS 32]
[ORG 0x1000]

%define KERNEL_TMP_ADDR     0x1200 ; temporary kernel location
%define KERNEL_FINAL_ADDR 0x100000 ; final kernel location

;; Secteur de boot - stage 2
;;
;;  Le boulot du "stage 2" est de copier le noyau en mémoire.

start:
	push eax ; kernel size received from stage 1
	call copyKernelToFinalLoc

	;; Jumping to kernel !
	jmp dword 0x8:0x100000

copyKernelToFinalLoc:
	push ebp
	mov ebp, esp

	mov eax, KERNEL_TMP_ADDR ; Source
	mov ebx, KERNEL_FINAL_ADDR ; Destintation
	mov ecx, [esp+4] ; retreiving the kernel size on the stack
	mov esi, 0

loop:
	cmp esi, ecx
	je end
	mov edx, [eax]
	mov [ebx], edx
	add eax, 1
	add ebx, 1
	add esi, 1
	jmp loop

end:
	leave
	ret

times 510-($-$$) db 144
dw 0xAA55