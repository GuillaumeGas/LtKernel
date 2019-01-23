[BITS 32]

%define SYSCALL_INTERRUPT 0x30

;; Voir liste des numéro de syscall dans kernel/user/syscalls.c
%define SYSCALL_PRINT         0x1
%define SYSCALL_SCAN          0x2
%define SYSCALL_EXIT          0x5
%define SYSCALL_OPEN_DIR      0x6
%define SYSCALL_READ_DIR      0x8
%define SYSCALL_GET_PROC_DIR  0xB
%define SYSCALL_SET_PROC_DIR  0xC
%define SYSCALL_LIST_PROCESS  255

global _print
global _scan
global _exit
global _openDir
global _readDir
global _getProcessDirectory
global _setCurrentDirectory
global _listProcess

_print:
    push ebp
    mov ebp, esp

    mov ebx, [ebp+8] ; on récupère le paramètre : pointeur sur la chaîne
    mov eax, SYSCALL_PRINT

    int SYSCALL_INTERRUPT

    leave
    ret

_scan:
	push ebp
    mov ebp, esp

    mov ebx, [ebp+8] ; on récupère le paramètre : pointeur sur la chaîne
    mov eax, SYSCALL_SCAN

    int SYSCALL_INTERRUPT

    leave
    ret

_exit:
	push ebp
    mov ebp, esp

    mov eax, SYSCALL_EXIT

    int SYSCALL_INTERRUPT

    leave
    ret

_openDir:
	push ebp
    mov ebp, esp

	mov ebx, [ebp+8]  ; on récupère le paramètre : pointeur sur la chaîne
	mov ecx, [ebp+12] ; on récupère le second param : pointeur sur le handle
    mov eax, SYSCALL_OPEN_DIR

    int SYSCALL_INTERRUPT

    leave
    ret

_readDir:
	push ebp
    mov ebp, esp

	mov ebx, [ebp+8]  ; on récupère le paramètre : handle sur rep
	mov ecx, [ebp+12] ; on récupère le second param : pointeur sur un DirEntry
    mov eax, SYSCALL_READ_DIR

    int SYSCALL_INTERRUPT

    leave
    ret

_getProcessDirectory:
	push ebp
    mov ebp, esp

	mov ebx, [ebp+8]  ; on récupère le paramètre : pointeur sur handle sur rep
    mov eax, SYSCALL_GET_PROC_DIR

    int SYSCALL_INTERRUPT

    leave
    ret

_setCurrentDirectory:
	push ebp
    mov ebp, esp

	mov ebx, [ebp+8]  ; on récupère le paramètre : handle sur rep
    mov eax, SYSCALL_SET_PROC_DIR

    int SYSCALL_INTERRUPT

    leave
    ret

_listProcess:
	push ebp
    mov ebp, esp

    mov eax, SYSCALL_LIST_PROCESS

    int SYSCALL_INTERRUPT

    leave
    ret