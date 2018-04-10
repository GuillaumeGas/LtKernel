[BITS 32]

extern memcopy
extern g_tss	
global asm_init_gdt
		
asm_init_gdt:
	;; on stock les info de tss dans le segment correspondant
	mov eax, g_tss
	mov edx, gdt_tss
	mov word [edx], ax
	shr eax, 16
	mov word [edx+6], ax

	mov eax, g_tss
	add eax, 104 		; struct tss size

	mov edx, [gdt_tss+2]
	mov word [edx], ax
	shr eax, 16
	mov byte [edx+4], al
	shr eax, 8
	mov byte [edx+7], al

	mov eax, 0		; adresse de destination
	mov ebx, gdt_begin	; adresse de la gdt locale
	mov ecx, 0x30		; taille en octets ; TODO : on pourrait calculer la taille...
	
	;; Stocke a l'adresse 0x0 la gdt
loop:	
	mov edx, [ebx]
	mov [eax], edx
	mov edx, [ebx+4]
	mov [eax+4], edx
	add ebx, 8
	add eax, 8
	cmp ecx, eax
	jge loop
			
	;; Load Gdt
	lgdt [gdt_ptr]

	;; Load tr
	mov ax, 0x28
	ltr ax
	
	;; on initialise les segments de donnees (0x10 car troisieme entree dans la GDT)
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	;; un jump pour mettre à jour le registre segment de code
	jmp 0x08:next
next:	
	ret

;; Gdt
gdt_begin:
	db 0, 0, 0, 0, 0, 0, 0, 0
gdt_cs:				; code segment
	db 0xFF, 0xFF, 0x0, 0x0, 0x0, 10011011b, 11011111b, 0x0
gdt_ds:				; data segment
	db 0xFF, 0xFF, 0x0, 0x0, 0x0, 10010011b, 11011111b, 0x0
gdt_ucs:
	db 0xFF, 0xFF, 0x0, 0x0, 0x0, 11111111b, 11001111b, 0x0
gdt_uds:
	db 0xFF, 0xFF, 0x0, 0x0, 0x0, 11110011b, 11001111b, 0x0
gdt_tss:
	db 0, 0, 0, 0, 0, 11101001b, 00000000b, 0x0
gdt_end:

gdt_ptr:
	dw 0x30                 ; limit : 16 bits (word)
	dd 0x0			; base : 32 bits (dword)

;; ;; Tss
;; tss_struct:
;; 	ISTRUCT Tss
	
;; 	at debug_flag dw 0
;; 	at io_map dw 0
;; 	at esp0 dd 0x20000 	; TODO créer un fichier qui rassemble toutes les constantes de base
;; 	at ss0 dw 0x10		; Le segment de pile est confondu avec celui de donneess

;; 	IEND
	
;; STRUCT Tss
;; previous_task:
;; 	resw
;; 	align 2
;; __previous_task_unused:
;; 	resw
;; 	align2
;; esp0:
;; 	resd
;; ss0:
;; 	resw
;; 	align2
;; __sso_unused:
;; 	resw
;; 	align 2
;; esp1:
;; 	resd
;; ss1:
;; 	resw
;; 	align 2
;; __ss1_unused:
;; 	resw
;; 	align 2
;; esp2:
;; 	resd
;; ss2:
;; 	resw
;; 	align 2
;; __ss2_unused:
;; 	resw
;; 	align 2
;; cr3:	resd
;; eip:	resd
;; eflags:	resd
;; eax:	resd
;; ecx:	resd
;; edx:	resd
;; ebx:	resd
;; esp:	resd
;; ebp:	resd
;; esi:	resd
;; edi:	resd
;; es:
;; 	resw
;; 	align 2
;; __es_unused:
;; 	resw
;; 	align 2
;; cs:
;; 	resw
;; 	align 2
;; __cs_unused:
;; 	resw
;; 	align 2
;; ss:
;; 	resw
;; 	align 2
;; __ss_unused:
;; 	resw
;; 	align 2
;; ds:
;; 	resw
;; 	align 2
;; __ds_unused:
;; 	resw
;; 	align 2
;; fs:
;; 	resw
;; 	align 2
;; __fs_unused:
;; 	resw
;; 	align 2
;; gs:
;; 	resw
;; 	align 2
;; __gs_unused:
;; 	resw
;; 	align 2
;; ldt_selector:
;; 	resw
;; 	align 2
;; __ldt_sel_unused:
;; 	resw
;; 	align 2
;; debug_flag:
;; 	resw
;; 	align 2
;; io_map:
;; 	resw
;; 	align 2
;; ENDSTRUCT
