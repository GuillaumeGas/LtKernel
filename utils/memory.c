#include "memory.h"

void memset (u8 byte, u8 * data, unsigned int size) {
    while ((size--) > 0)
	*(data--) = byte;
}

void memcopy (u8 * src, u8 * dst, unsigned int size) {
    while ((size--) > 0)
	*(dst++) = *(src++);
}
