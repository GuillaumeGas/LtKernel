#include <kernel/kernel.h>
#include <kernel/init/vmm.h>
#include <kernel/init/heap.h>
#include <kernel/lib/panic.h>
#include <kernel/lib/stdlib.h>
#include <kernel/lib/stdio.h>

static void * _kmalloc(MemBlock * block, int size);
static void _splitBlock(MemBlock * block, int size);
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
			u8 * new_page = (u8*)get_free_page();

			if (new_page == NULL)
				panic(MEMORY_FULL);

			pd0_add_page((u8 *)g_heap, new_page, IN_MEMORY | WRITEABLE);

			heap += PAGE_SIZE;
		}
		g_heap = (MemBlock *)heap;

		newBlock->size = n * PAGE_SIZE;
		newBlock->state = BLOCK_FREE;
		
		mmset((u8 *)(&(newBlock->data)), newBlock->size - BLOCK_HEADER_SIZE, 0);

		return newBlock;
	}
}

void * kmalloc(int size)
{
	void * res = NULL;
	res = _kmalloc((MemBlock*)g_kernelInfo.heapBase_v, size + BLOCK_HEADER_SIZE);
	_kdefrag();
	return res;
}

void kfree(void * ptr)
{
	MemBlock * block = (MemBlock*)((int)ptr - BLOCK_HEADER_SIZE);
	block->state = BLOCK_FREE;
	mmset((u8 *)(&(block->data)), block->size - BLOCK_HEADER_SIZE, 0);
}

static void * _kmalloc(MemBlock * block, int size)
{
	void * res_ptr = NULL;

	while (block <= g_heap && res_ptr == NULL)
	{
		if (block->state == BLOCK_USED || size > block->size)
		{
			block = block + block->size;
			continue;
		}

		if ((block->size - size) >= (int)(BLOCK_HEADER_SIZE + MINIMAL_BLOCK_SIZE))
			_splitBlock(block, size);

		res_ptr = &(block->data);
	}

	if (res_ptr == NULL)
	{
		block = ksbrk(size / DEFAULT_BLOCK_SIZE);

		res_ptr = _kmalloc(block, size);
	}

	block->state = BLOCK_USED;

	return res_ptr;
}

static void _splitBlock(struct mem_block * block, int size)
{
	struct mem_block * second_block = block + size;
	second_block->size = block->size - size;
	second_block->state = BLOCK_FREE;

	if (block == g_heap)
		g_heap = second_block;

	block->size = size;
}

static void _kdefrag()
{
	struct mem_block * block = g_heap;

	while (block < g_heap)
	{
		struct mem_block * next = block + block->size;

		if (block->state == BLOCK_FREE && next->state == BLOCK_FREE)
		{
			block->size += next->size;
			mmset((u8 *)(&(block->data)), 0, block->size - BLOCK_HEADER_SIZE);
			if (next == g_heap)
				g_heap = block;
		}
		else
		{
			block += block->size;
		}
	}
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