#define __HEAP__
#include "heap.h"

#include <kernel/kernel.h>
#include <kernel/lib/kmalloc.h>
#include <kernel/lib/stdio.h>
#include <kernel/lib/panic.h>

/*
	Initialise le tas avec un un bloc (taille d'une page)
*/
void init_heap()
{
	g_heap = (MemBlock *)g_kernelInfo.heapBase_v;

	ksbrk(1);

	kprint("[Kernel] Kernel heap initialized\n");
}

/*
	Initialise le tas de pages
*/
void init_page_heap()
{
	MemPageBlock * tmp = NULL;
	MemPageBlock * prev = NULL;

	g_page_heap = (MemPageBlock *)kmalloc(sizeof(MemPageBlock));
	g_page_heap->available = BLOCK_FREE;
	g_page_heap->next = NULL;
	g_page_heap->prev = NULL;
	g_page_heap->v_page_addr = (u32 *)g_kernelInfo.pagesHeapBase_v;

	tmp = g_page_heap;

	while (tmp->v_page_addr < (u32 *)g_kernelInfo.pagesHeapLimit_v)
	{
        tmp->next = (MemPageBlock *)kmalloc(sizeof(MemPageBlock));

        prev = tmp;
		tmp = tmp->next;

		tmp->v_page_addr = (u32 *)((unsigned int)prev->v_page_addr + PAGE_SIZE);

		tmp->available = BLOCK_FREE;
		tmp->prev = prev;
		tmp->next = NULL;
	}

	kprint("[Kernel] Kernel pages heap initialized\n");
}

void CleanPageHeap()
{
    MemPageBlock * block = g_page_heap;

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
	kprint(" - Number of kmalloc() : %d\n", g_kmalloc_count);
	kprint(" - NUmber of kfree()   : %d\n", g_kfree_count);
	kprint(" - Diff                : %d\n", g_kmalloc_count - g_kfree_count);
}