[BITS 16]
[ORG 0x0]

%define DATA_ADDR        0x07C0	; data segment
%define STACK_ADDR       0x8000	; stack segment
%define STACK_PTR_OFFSET 0xF000	; stack pointer
%define STAGE_2_ADDR     0x0100 ; stage two loaded at 0x1000 (selector 0100:0000 -> 0x1000 (physical mem))
%define NB_SECTOR            50 ; sectors to read count
%define SECTOR_SIZE			512 ; a sector size in bytes
%define NUM_SECTOR            2 ; sector
%define BOOT_DRIVE            0 ; drive (for int 0x13)
	
;; Secteur de boot - stage 1
;;
;;  Le principal boulot du "stage 1" du secteur de boot est 
;;      - d'initialiser la GDT
;;      - de charger le stage 2 et le noyau en mémoire.
;;  Le "stage 2", exécuté en 32bits, va permettre le chargement du noyau à une adresse par accessible en 16bits.
;;
;;  Effectue les initialisations de base de la mémoire/registres
;;  Charge le stage 2 et le noyau en 0x1000
;;  Passe en mode protégé (comme ça on se retrouve en 32bits dans le stage 2)
;;  On donne la main au stage 2

	jmp start

start:
	;; Segments init
	mov ax, DATA_ADDR
	mov ds, ax
	mov es, ax
	mov ax, STACK_ADDR
	mov ss, ax
	mov sp, STACK_PTR_OFFSET

	call initGdt
	call loadStage2AndKernel
	call switchToProtectedMode

	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	mov esp, 0x9F000

	;; Passing argment to stage 2 : "size" of the kernel that will be copied at address 0x100000 (see stage 2)
	mov eax, NB_SECTOR
	sub eax, 1 ; because NB_SECTOR includes the stage 2
	mov edx, SECTOR_SIZE
	mul edx ;; RESULT THAT MUST BE READ IN STAGE 2

	;; Jumping to stage 2 !
	jmp dword 0x8:0x1000
	
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

;; ###### GDT ######
initGdt:
	push bp
	mov bp, sp

	mov si, loadGdtMsg
	call print

	;; Loading gdt
	;; Gdt size
	mov ax, gdt_end
	mov bx, gdt_begin
	sub ax, bx
	mov word [gdt_ptr], ax

	;; Gdt physical address
	xor eax, eax
	xor ebx, ebx
	mov ax, ds
	shl eax, 4
	mov bx, gdt_begin
	add eax, ebx
	mov dword [gdt_ptr+2], eax

	;; Clear interrupt flag
	cli

	;; Load Gdt
	lgdt [gdt_ptr]

	;; Set interrupt flag
	sti

	mov si, okMsg
	call print

	leave
	ret

	;; ###### LOADS STAGE 2 & KERNEL ######
loadStage2AndKernel:
	mov si, loadStage2Msg
	call print

	;; Loads the stage 2
	mov ah, 0
	mov dl, BOOT_DRIVE
	int 0x13

	push es
	mov ax, STAGE_2_ADDR
	mov es, ax
	mov bx, 0
	
	mov ah, 0x02		; we read sectors from drive
	mov al, NB_SECTOR
	mov ch, 0
	mov cl, NUM_SECTOR
	mov dh, 0
	mov dl, BOOT_DRIVE
	int 0x13
	pop es

	mov si, okMsg
	call print

	;; ###### PROTECTED MODE ######
switchToProtectedMode:
	mov si, protectedModeMsg
	call print

	cli

	;; Enable protected mode
	;; xor eax, eax
	;; mov eax, 1
	mov eax, cr0
	or ax, 1
	mov cr0, eax

	;; because of the documentation...
	jmp next
next:

	;; Variables
	loadGdtMsg db "[STAGE 1] Loading GDT...", 0
	loadStage2Msg db "[STAGE 1] Loading stage 2...", 0
	protectedModeMsg db "[STAGE 1] Switching to protected mode...", 0
	loadKernelMsg db "[STAGE 2] Loading kernel...", 0
	okMsg db "OK", 13, 10, 0

	;; Gdt
gdt_begin:
	db 0, 0, 0, 0, 0, 0, 0, 0
gdt_cs:				; code segment
	db 0xFF, 0xFF, 0x0, 0x0, 0x0, 10011011b, 11011111b, 0x0
gdt_ds:				; data segment
	db 0xFF, 0xFF, 0x0, 0x0, 0x0, 10010011b, 11011111b, 0x0
gdt_end:

gdt_ptr:
	dw 0x0000       ; limit : 16 bits (word)
	dd 0			; base : 32 bits (dword)
	
times 510-($-$$) db 144
dw 0xAA55
