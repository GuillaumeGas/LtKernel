#include <kernel/kernel.h>
#include <kernel/init/vmm.h>
#include <kernel/init/heap.h>
#include <kernel/lib/panic.h>
#include <kernel/lib/stdlib.h>
#include <kernel/lib/stdio.h>

#include <kernel/logger.h>
#define KLOG(LOG_LEVEL, format, ...) KLOGGER("LIB", LOG_LEVEL, format, ##__VA_ARGS__)

#include <kernel/debug/debug.h>

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
	if (n <= 0)
	{
		KLOG(LOG_ERROR, "n <= 0");
		return NULL;
	}

	// On ne doit pas dépasser la limite de l'espace réservé au tas
	if ((u32)gHeap + (n * PAGE_SIZE) > gKernelInfo.vHeapLimit)
	{
		KLOG(LOG_ERROR, "Kernel heap limit reached");
		panic(HEAP_LIMIT);
		return NULL;
	}
	else
	{
		unsigned int i = 0;
		MemBlock * newBlock = gHeap;
        u32 heap = (u32)gHeap;

		for (; i < n; i++)
		{
			u8 * new_page = (u8*)GetFreePage();

			if (new_page == NULL)
			{
				KLOG(LOG_ERROR, "Couldn't find a free page");
				panic(MEMORY_FULL);
			}

			AddPageToKernelPageDirectory((u8 *)heap, new_page, PAGE_PRESENT | PAGE_WRITEABLE);

			heap += (u32)PAGE_SIZE;
		}

        gHeap = (MemBlock *)heap;

		newBlock->size = n * PAGE_SIZE;
		newBlock->state = BLOCK_FREE;

		MmSet((u8 *)(&(newBlock->data)), 0, newBlock->size - BLOCK_HEADER_SIZE);

		return newBlock;
	}
}

void * kmalloc(int size)
{
	if (size <= 0)
	{
		KLOG(LOG_WARNING, "Kernel allocation with size <= 0");
		return NULL;
	}

	void * res = NULL;
    res = _kmalloc((MemBlock*)gKernelInfo.vHeapBase, size + BLOCK_HEADER_SIZE);
	gKMallocCount++;
	return res;
}

void kfree(void * ptr)
{
	if (ptr == NULL)
	{
		KLOG(LOG_ERROR, "Trying to free a NULL pointer");
		return;
	}

	MemBlock * block = (MemBlock*)((u32)ptr - BLOCK_HEADER_SIZE);
	block->state = BLOCK_FREE;
	MmSet((u8 *)(&(block->data)), 0, block->size - BLOCK_HEADER_SIZE);
	gKFreeCount++;
    _kdefrag();
}

static unsigned int _mod(unsigned int a, unsigned int b)
{
	while (a > b)
		a -= b;
	return (a > 0) ? 1 : 0;
}

static void * _kmalloc(MemBlock * block, int size)
{
	void * res_ptr = NULL;

	while (block < gHeap && res_ptr == NULL)
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
		{
			unsigned int usize = (unsigned int)size;
			const unsigned int ubsize = (unsigned int)DEFAULT_BLOCK_SIZE;
			unsigned int n = usize / ubsize;
			if (_mod(usize, ubsize) > 0)
			{
				n++;
			}
			block = ksbrk(n);
		}
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
	MemBlock * block = (MemBlock *)gKernelInfo.vHeapBase;

	while (block < gHeap)
	{
		MemBlock * next = (MemBlock *)((unsigned int)block + block->size);

		if (next < gHeap)
		{
			if (block->state == BLOCK_FREE && next->state == BLOCK_FREE)
			{
				block->size += next->size;
				MmSet((u8 *)(&(block->data)), 0, block->size - BLOCK_HEADER_SIZE);
			}
		}
		block = (MemBlock *)((unsigned int)(block) + block->size);
	}
}

void dumpHeap()
{
	MemBlock * block = (MemBlock *)gKernelInfo.vHeapBase;
	int i = 0;
	kprint("== Heap Dump ==\n\n");
	while (block < gHeap)
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
	MemPageBlock * block = gPageHeap;

	newPage.pAddr = (u32 *)GetFreePage();

	if (newPage.pAddr == NULL)
	{
		KLOG(LOG_ERROR, "Couldn't find a free page");
		panic(MEMORY_FULL);
	}

	while (block != NULL)
	{
		if (block->available == BLOCK_FREE)
		{
			block->available = BLOCK_USED;
			newPage.vAddr = block->vPageAddr;
			break;
		}

		block = block->next;
	}

	if (newPage.vAddr == NULL)
	{
		KLOG(LOG_ERROR, "Couldn't find an available virtual address");
		panic(VIRTUAL_MEMORY_FULL);
	}

	AddPageToKernelPageDirectory((u8 *)newPage.vAddr, (u8 *)newPage.pAddr, PAGE_PRESENT | PAGE_WRITEABLE);

	return newPage;
}

// Virer la page du répertoire du noyau
void PageFree(void * ptr)
{
	MemPageBlock * block = gPageHeap;

	if (ptr == NULL)
	{
		KLOG(LOG_ERROR, "Trying to free a page will NULL address");
		return;
	}

	while (block != NULL)
	{
		if (block->vPageAddr == ptr)
		{
			block->available = BLOCK_FREE;
			block = NULL;
			ReleasePage(GetPhysicalAddress(ptr));
		}
	}
}