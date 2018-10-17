#include <kernel/lib/types.h>
#include <kernel/lib/stdlib.h>

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
	if (src == NULL || size == 0)
		return;

	while ((size--) > 0)
		*(dst++) = *(src++);
}

void mmset(u8 * src, u8 byte, unsigned int size)
{
	if (src == NULL || size == 0)
		return;

	while ((size--) > 0)
		*(src++) = byte;
}

void strcpy(const char * src, char * dst)
{
	if (src == NULL || dst == NULL)
		return;

	while (*src != '\0')
		*(dst++) = *(src++);
}