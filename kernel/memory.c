#include "memory.h"

void memcopy (u8 * src, u8 * dst, unsigned int size)
{
    while ((size--) > 0)
	*(dst++) = *(src++);
}
