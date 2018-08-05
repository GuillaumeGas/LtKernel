#pragma once

#include <kernel/lib/types.h>

void mmcopy (u8 * src, u8 * dst, unsigned int size);
void mmset (u8 * src, u8 byte, u32 size);