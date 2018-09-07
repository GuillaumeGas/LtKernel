#pragma once

#define FlagOn(a, b) ((a & b) != 0)
#define NULL 0

void pause ();

void mmcopy(u8 * src, u8 * dst, unsigned int size);
void mmset(u8 * src, u8 byte, u32 size);