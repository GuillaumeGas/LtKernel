#include <kernel/kernel.h>
#include <kernel/init/vmm.h>
#include <kernel/init/heap.h>
#include <kernel/lib/panic.h>
#include <kernel/lib/stdlib.h>
#include <kernel/lib/stdio.h>

/*
	
	TODO : g�rer les cas d'erreur (tailles n�gatives, pointeurs null...)
		   remplacer les int par unsigned int ?
	
*/

static void * _kmalloc(MemBlock * block, int size);
static void _splitBlock(MemBlock * block, unsigned int size);
static void _kdefrag();

/*
	Agrandi le tas

	Va chercher n pages en m�moire
	Les ajoute ces pages � l'espace d'adressage du noyau
	Les nouveaux bloques sont initialis�s
*/
MemBlock * ksbrk(int n)
{
	// On ne doit pas d�passer la limite de l'espace r�serv� au tas
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
			u8 * new_page = (u8*)get_free_page();

			if (new_page == NULL)
				panic(MEMORY_FULL);

			pd0_add_page((u8 *)newBlock, new_page, IN_MEMORY | WRITEABLE);

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
	/*MemBlock * block = g_kernelInfo.pagesHeapBase_v;

	while (block < g_heap)
	{
		MemBlock * next = block + block->size;

		if (next->size > 0)
		{
			if (block->state == BLOCK_FREE && next->state == BLOCK_FREE)
			{
				block->size += next->size;
				kprint("block->size : %d | %d\n", (unsigned int)block->size, (int)block->size);
				mmset2((u8 *)(&(block->data)), 0, block->size - BLOCK_HEADER_SIZE);
			}
		}
		else
		{
			kprint("What ?? addr : %x\n");
		}
		block += block->size;
	}*/
}

void dumpHeap()
{
	MemBlock * block = (MemBlock *)g_kernelInfo.heapBase_v;
	int i = 0;
	kprint("== Heap Dump ==\n\n");
	while (block < g_heap && i < 10)
	{
		kprint("[%d] Size : %d, Addr : %x, ", i++, block->size, block);
		if (block->state == BLOCK_FREE)
			kprint("FREE\n");
		else
			kprint("\n");
		block = block + block->size;
	}
	kprint("\n");
}

Page page_alloc()
{
	Page newPage = {0};
	struct mem_pblock * block = g_page_heap;

	newPage.p_addr = (u32 *)get_free_page();
	
	if (newPage.p_addr == NULL)
		panic(MEMORY_FULL);

	while (block != NULL)
	{
		if (block->available == BLOCK_FREE)
		{
			block->available = BLOCK_USED;
			newPage.v_addr = block->v_page_addr;
		}

		block = block->next;
	}

	if (newPage.v_addr == NULL)
		panic(VIRTUAL_MEMORY_FULL);

	return newPage;
}

void page_free(void * ptr)
{
	struct mem_pblock * block = g_page_heap;

	while (block != NULL)
	{
		if (block->v_page_addr == ptr)
		{
			block->available = BLOCK_FREE;
			block = NULL;
			release_page(get_p_addr(ptr));
		}
	}
}