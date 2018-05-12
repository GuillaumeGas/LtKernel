#ifndef __DEF_MEMORY__
#define __DEF_MEMORY__

#include "types.h"

void mmcopy (u8 * src, u8 * dst, unsigned int size);
void mmset (u8 * src, u8 byte, u32 size);

#endif
