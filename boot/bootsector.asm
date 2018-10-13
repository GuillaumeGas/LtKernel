MBALIGN  equ  1 << 0            ; Align loaded modules on page boundaries
MEMINFO  equ  1 << 1            ; Provide memory map
FLAGS    equ  MBALIGN | MEMINFO ; This is the Multiboot 'flag' field
MAGIC    equ  0x1BADB002        ; 'magic number' lets bootloader find the header
CHECKSUM equ -(MAGIC + FLAGS)   ; Checksum of above, to prove we are multiboot

section .multiboot_header
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

section .text
extern start_kmain
global _start

_start:
        push ebx
        call start_kmain

        cli ; stop interrupts
        hlt ; halt the CPU