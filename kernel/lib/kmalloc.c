#include <kernel/kernel.h>
#include <kernel/init/vmm.h>
#include <kernel/init/heap.h>
#include <kernel/lib/panic.h>
#include <kernel/lib/stdlib.h>
#include <kernel/lib/stdio.h>

/*
	
	TODO : gérer les cas d'erreur (tailles négatives, pointeurs null...)
		   remplacer les int par unsigned int ?
	
*/

static void * _kmalloc(MemBlock * block, int size);
static void _splitBlock(MemBlock * block, unsigned int size);
static void _kdefrag();

/*
	Agrandi le tas

	Va chercher n pages en mémoire
	Les ajoute ces pages à l'espace d'adressage du noyau
	Les nouveaux bloques sont initialisés
*/
MemBlock * ksbrk(int n)
{
	// On ne doit pas dépasser la limite de l'espace réservé au tas
	if ((u32)g_heap + (n * PAGE_SIZE) > g_kernelInfo.heapLimit_v)
	{
		panic(HEAP_LIMIT);
		return NULL;
	}
	else
	{
		unsigned int i = 0;
		MemBlock * newBlock = g_heap;
        u32 heap = (u32)g_heap;

		for (; i < n; i++)
		{
			u8 * new_page = (u8*)GetFreePage();

			if (new_page == NULL)
				panic(MEMORY_FULL);

			AddPageToKernelPageDirectory((u8 *)newBlock, new_page, PAGE_PRESENT | PAGE_WRITEABLE);

			heap += (u32)PAGE_SIZE;
		}

        g_heap = (MemBlock *)heap;

		newBlock->size = n * PAGE_SIZE;
		newBlock->state = BLOCK_FREE;

		mmset((u8 *)(&(newBlock->data)), 0, newBlock->size - BLOCK_HEADER_SIZE);

		return newBlock;
	}
}

void * kmalloc(int size)
{
	void * res = NULL;
    res = _kmalloc((MemBlock*)g_kernelInfo.heapBase_v, size + BLOCK_HEADER_SIZE);
	g_kmalloc_count++;
	return res;
}

void kfree(void * ptr)
{
	MemBlock * block = (MemBlock*)((u32)ptr - BLOCK_HEADER_SIZE);
	block->state = BLOCK_FREE;
	mmset((u8 *)(&(block->data)), 0, block->size - BLOCK_HEADER_SIZE);
	g_kfree_count++;
    _kdefrag();
}

static void * _kmalloc(MemBlock * block, int size)
{
	void * res_ptr = NULL;

	while (block < g_heap && res_ptr == NULL)
	{
		if (block->state == BLOCK_USED || size > block->size)
		{
			block = (MemBlock *)((unsigned int)block + block->size);
			continue;
		}

		if ((block->size - size) >= (int)(BLOCK_HEADER_SIZE + MINIMAL_BLOCK_SIZE))
			_splitBlock(block, size);

		res_ptr = &(block->data);
	}

	if (res_ptr == NULL)
	{
        if (size > DEFAULT_BLOCK_SIZE)
            block = ksbrk(size / DEFAULT_BLOCK_SIZE);
        else
            block = ksbrk(1);

		res_ptr = _kmalloc(block, size);
	}

	block->state = BLOCK_USED;

	return res_ptr;
}

static void _splitBlock(MemBlock * block, unsigned int size)
{
	MemBlock * second_block = (MemBlock *)((unsigned int)block + size);
	second_block->size = block->size - size;
	second_block->state = BLOCK_FREE;
	block->size = size;
}

static void _kdefrag()
{
	MemBlock * block = (MemBlock *)g_kernelInfo.heapBase_v;

	while (block < g_heap)
	{
		MemBlock * next = (MemBlock *)((unsigned int)block + block->size);

		if (next < g_heap)
		{
			if (block->state == BLOCK_FREE && next->state == BLOCK_FREE)
			{
				block->size += next->size;
				mmset((u8 *)(&(block->data)), 0, block->size - BLOCK_HEADER_SIZE);
			}
		}
		block = (MemBlock *)((unsigned int)(block) + block->size);
	}
}

void dumpHeap()
{
	MemBlock * block = (MemBlock *)g_kernelInfo.heapBase_v;
	int i = 0;
	kprint("== Heap Dump ==\n\n");
	while (block < g_heap)
	{
		kprint("[%d] Size : %d, Addr : %x, ", i++, block->size, block);
		if (block->state == BLOCK_FREE)
			kprint("FREE\n");
		else
			kprint("\n");
		block = (MemBlock *)((unsigned int)block + block->size);
	}
	kprint("\n");
}

// Pour le moment on se contente d'ajouter la page dans pd0, je sais pas si c'est suffisant, est ce qu'on est sûr que le pd0
// utilisé au moment de l'appel est le bon à chaque fois qu'on a besoin de cette fonction ?
Page PageAlloc()
{
	Page newPage = {0};
	MemPageBlock * block = g_page_heap;

	newPage.p_addr = (u32 *)GetFreePage();

	if (newPage.p_addr == NULL)
		panic(MEMORY_FULL);

	while (block != NULL)
	{
		if (block->available == BLOCK_FREE)
		{
			block->available = BLOCK_USED;
			newPage.v_addr = block->v_page_addr;
			break;
		}

		block = block->next;
	}

	if (newPage.v_addr == NULL)
		panic(VIRTUAL_MEMORY_FULL);

	AddPageToKernelPageDirectory((u8 *)newPage.v_addr, (u8 *)newPage.p_addr, PAGE_PRESENT | PAGE_WRITEABLE);

	return newPage;
}

// Virer la page du répertoire du noyau
void PageFree(void * ptr)
{
	MemPageBlock * block = g_page_heap;

	while (block != NULL)
	{
		if (block->v_page_addr == ptr)
		{
			block->available = BLOCK_FREE;
			block = NULL;
			ReleasePage(GetPhysicalAddress(ptr));
		}
	}
}