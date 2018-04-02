[BITS 32]
global init_idt

extern _asm_default_isr
extern _asm_clock_isr	
	
init_idt:
	mov eax, _asm_default_isr
	mov ebx, idt_desc_default ; on commence par remplir le descripteur
	mov word [ebx], ax
	shr eax, 16		; obligé de faire la copie de l'adresse en deux fois car 16 bits copiés à un endroit et le reste copié à un autre
	mov [ebx+6], ax
	
	mov ebx, 0x800 		; adresse où on va copier l'idt
	mov eax, 0x908		; adresse de fin (+33 entrees dans la table pour l'instant)
	;; donc 32 pour les exceptions et une pour l'IRQ 0 (horloge)

	;; On remplie tous les descripteurs par une routine par défaut
loop:	
	mov edx, [idt_desc_default]
	mov [ebx], edx
	mov edx, [idt_desc_default+4]
	mov [ebx+4], edx
	add ebx, 8
	cmp eax, ebx
	jge loop
	
	;; On s'occupe de renseigner l'isr de l'IRQ 0 (horloge)
	mov eax, _asm_clock_isr
	mov ebx, idt_desc_clock	; on commence par remplir le descripteur
	mov word [ebx], ax
	mov [ebx+6], eax

	mov eax, 0x900
	mov ebx, [idt_desc_clock]
	mov ecx, [idt_desc_clock+4]
	mov [eax], ebx
	mov [ebx+4], ecx

	lidt [idt_ptr]
	ret
	
idt_desc_default:
	db 0x0, 0x0, 0x8, 0x0, 0x0, 10001110b, 0x0, 0x0
	db 0, 0, 0, 0, 0, 0, 0, 0
	
idt_desc_clock:
	db 0x0, 0x0, 0x8, 0x0, 0x0, 10001110b, 0x0, 0x0
	db 0, 0, 0, 0, 0, 0, 0, 0	

idt_ptr:
	dw 0x108
	dd 0x800
