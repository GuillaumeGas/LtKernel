#pragma once

#include <kernel/lib/types.h>
#include <kernel/lib/stdlib.h>

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
	u32 * v_page_addr;
	struct mem_pblock * prev;
	struct mem_pblock * next;
};
typedef struct mem_pblock MemPageBlock;

void init_heap();
void init_page_heap();
void CleanPageHeap();

void CheckHeap();

#ifdef __HEAP__
MemBlock * g_heap = NULL; // Pointe juste après le dernier bloc créé par ksbrk(), limite courante du tas...
int g_kmalloc_count = 0;
int g_kfree_count = 0;
MemPageBlock * g_page_heap = NULL;
#else
extern MemBlock * g_heap;
extern MemPageBlock * g_page_heap;
extern int g_kmalloc_count;
extern int g_kfree_count;
#endif
