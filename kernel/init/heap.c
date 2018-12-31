#define __HEAP__
#include "heap.h"

#include <kernel/kernel.h>

#include <kernel/lib/kmalloc.h>
#include <kernel/lib/stdio.h>
#include <kernel/lib/panic.h>

#include <kernel/logger.h>
#define KLOG(LOG_LEVEL, format, ...) KLOGGER("HEAP", LOG_LEVEL, format, ##__VA_ARGS__)

/*
	Initialise le tas avec un un bloc (taille d'une page)
*/
void HeapInit()
{
	gHeap = (MemBlock *)gKernelInfo.vHeapBase;

	ksbrk(1);
}

/*
	Initialise le tas de pages
*/
void PageHeapInit()
{
	MemPageBlock * tmp = NULL;
	MemPageBlock * prev = NULL;

	gPageHeap = (MemPageBlock *)kmalloc(sizeof(MemPageBlock));

    if (gPageHeap == NULL)
    {
        KLOG(LOG_ERROR, "Couldn't allocate %d bytes for page heap initialization", sizeof(MemPageBlock));
        Pause();
    }

	gPageHeap->available = BLOCK_FREE;
	gPageHeap->next = NULL;
	gPageHeap->prev = NULL;
	gPageHeap->vPageAddr = (u32 *)gKernelInfo.vPagesHeapBase;

	tmp = gPageHeap;

	while (tmp->vPageAddr < (u32 *)gKernelInfo.vPagesHeapLimit)
	{
        tmp->next = (MemPageBlock *)kmalloc(sizeof(MemPageBlock));

        if (tmp->next == NULL)
        {
            KLOG(LOG_ERROR, "Couldn't allocate %d bytes for page heap initialization (vPageAddr : 0x%x, vPagesHeapLimit : 0x%x)", sizeof(MemPageBlock), tmp->vPageAddr, gKernelInfo.vPagesHeapLimit);
            Pause();
        }

        prev = tmp;
		tmp = tmp->next;

		tmp->vPageAddr = (u32 *)((unsigned int)prev->vPageAddr + PAGE_SIZE);

		tmp->available = BLOCK_FREE;
		tmp->prev = prev;
		tmp->next = NULL;
	}
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

    if ((gKMallocCount - gKFreeCount) > 0)
        KLOG(LOG_WARNING, "Memory leaks !");
}