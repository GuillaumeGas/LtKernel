[BITS 32]
global init_idt

extern _asm_default_isr
extern _asm_clock_isr	
extern _asm_keyboard_isr
	
init_idt:
	mov eax, _asm_default_isr
	mov ebx, idt_empty_descriptor ; on commence par remplir le descripteur
	mov word [ebx], ax
	shr eax, 16		; obligé de faire la copie de l'adresse en deux fois car 16 bits copiés à un endroit et le reste copié à un autre
	mov [ebx+6], ax
	
	mov ebx, 0x800 		; adresse où on va copier l'idt
	mov eax, 0xB20		; adresse de fin (0xFF entrées)
	;; donc 32 pour les exceptions et une pour l'IRQ 0 (horloge)

	;; On remplie tous les descripteurs par une routine par défaut
loop:	
	mov edx, [idt_empty_descriptor]
	mov [ebx], edx
	mov edx, [idt_empty_descriptor+4]
	mov [ebx+4], edx
	add ebx, 8
	cmp eax, ebx
	jge loop

	;; IRQ 0 (horloge)
	push _asm_clock_isr
	push idt_empty_descriptor
	push 0x900
	call add_entry
	add esp, 12

	;; IRQ 1 (clavier)
	push _asm_keyboard_isr
	push idt_empty_descriptor
	push 0x908
	call add_entry
	add esp, 12
	
	lidt [idt_ptr]

	ret

add_entry:
	push ebp
	mov ebp, esp

	mov ecx, [ebp + 16]
	mov ebx, [ebp + 12]
	mov word [ebx], cx
	shr ecx, 16
	mov [ebx+6], cx

	mov eax, [ebp + 8]
	mov ecx, [ebx+4]
	mov ebx, [ebx]
	mov [eax], ebx
	mov [eax+4], ecx

	mov esp, ebp
	pop ebp
	
	ret
	
idt_empty_descriptor:
	db 0x0, 0x0, 0x8, 0x0, 0x0, 10001110b, 0x0, 0x0
	db 0, 0, 0, 0, 0, 0, 0, 0
	
idt_ptr:
	dw 0x320
	dd 0x800
