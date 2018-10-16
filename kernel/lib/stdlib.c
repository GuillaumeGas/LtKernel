#include <kernel/lib/types.h>

void hlt()
{
	asm("hlt");
}

void pause ()
{
    while (1) {}
}

void mmcopy(u8 * src, u8 * dst, unsigned int size)
{
	while ((size--) > 0)
		*(dst++) = *(src++);
}

void mmset(u8 * src, u8 byte, u32 size)
{
	while ((size--) > 0)
		*(src++) = byte;
}

void strcpy(const char * src, char * dst)
{
	while (*src != '\0')
		*(dst++) = *(src++);
}