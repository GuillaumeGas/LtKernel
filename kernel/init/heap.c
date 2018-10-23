#define __HEAP__
#include "heap.h"

#include <kernel/kernel.h>

#include <kernel/lib/kmalloc.h>
#include <kernel/lib/stdio.h>
#include <kernel/lib/panic.h>

/*
	Initialise le tas avec un un bloc (taille d'une page)
*/
void HeapInit()
{
	gHeap = (MemBlock *)gKernelInfo.vHeapBase;

	ksbrk(1);

	kprint("[Kernel] Kernel heap initialized\n");
}

/*
	Initialise le tas de pages
*/
void PageHeapInit()
{
	MemPageBlock * tmp = NULL;
	MemPageBlock * prev = NULL;

	gPageHeap = (MemPageBlock *)kmalloc(sizeof(MemPageBlock));
	gPageHeap->available = BLOCK_FREE;
	gPageHeap->next = NULL;
	gPageHeap->prev = NULL;
	gPageHeap->vPageAddr = (u32 *)gKernelInfo.vPagesHeapBase;

	tmp = gPageHeap;

	while (tmp->vPageAddr < (u32 *)gKernelInfo.vPagesHeapLimit)
	{
        tmp->next = (MemPageBlock *)kmalloc(sizeof(MemPageBlock));

        prev = tmp;
		tmp = tmp->next;

		tmp->vPageAddr = (u32 *)((unsigned int)prev->vPageAddr + PAGE_SIZE);

		tmp->available = BLOCK_FREE;
		tmp->prev = prev;
		tmp->next = NULL;
	}

	kprint("[Kernel] Kernel pages heap initialized\n");
}

void CleanPageHeap()
{
    MemPageBlock * block = gPageHeap;

    while (block != NULL)
    {
        MemPageBlock * next = block->next;
        kfree(block);
        block = next;
    }
}

void CheckHeap()
{
	kprint("== Heap Check ==\n");
	kprint(" - Number of kmalloc() : %d\n", gKMallocCount);
	kprint(" - NUmber of kfree()   : %d\n", gKFreeCount);
	kprint(" - Diff                : %d\n", gKMallocCount - gKFreeCount);
}