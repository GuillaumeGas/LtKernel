#pragma once

#include <kernel/lib/types.h>
#include <kernel/lib/stdlib.h>

#define BLOCK_HEADER_SIZE sizeof(int)
#define DEFAULT_BLOCK_SIZE PAGE_SIZE - BLOCK_HEADER_SIZE
#define DEFAULT_BLOCK_SIZE_WITH_HEADER DEFAULT_BLOCK_SIZE + BLOCK_HEADER_SIZE

#define MINIMAL_BLOCK_SIZE 4
#define BLOCK_FREE 0
#define BLOCK_USED 1

struct mem_block
{
	unsigned int size : 31; // 31 bits pour la taille, 1 bit pour indiquer si le bloc est libre (0) ou non (1)
	unsigned int state : 1;
	void * data;
};
typedef struct mem_block MemBlock;

struct mem_pblock
{
	u8 available;
	u32 * vPageAddr;
	struct mem_pblock * prev;
	struct mem_pblock * next;
};
typedef struct mem_pblock MemPageBlock;

void HeapInit();
void PageHeapInit();
void CleanPageHeap();

void CheckHeap();

#ifdef __HEAP__
MemBlock * gHeap = NULL; // Pointe juste après le dernier bloc créé par ksbrk(), limite courante du tas...
int gKMallocCount = 0;
int gKFreeCount = 0;
MemPageBlock * gPageHeap = NULL;
#else
extern MemBlock * gHeap;
extern MemPageBlock * gPageHeap;
extern int gKMallocCount;
extern int gKFreeCount;
#endif
