ENTRY(_start)

SECTIONS
{
    . = 0x100000;

    .text ALIGN(4K):
    {
        KEEP(*(.multiboot_header))
        KEEP(*(.text))
    }
 
    /* Read-only data. */
    .rodata ALIGN(4K):
    {
        KEEP(*(.rodata))
    }
 
    /* Read-write data (initialized) */
    .data ALIGN(4K):
    {
        KEEP(*(.data))
    }
 
    /* Read-write data (uninitialized) and stack */
    .bss ALIGN(4K):
    {
        KEEP(*(COMMON))
        KEEP(*(.bss))
    }
 
    /* The compiler may produce other sections, by default it will put them in
       a segment with the same name. Simply add stuff here as needed. */
}