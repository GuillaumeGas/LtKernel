#include <kernel/init/memory.h>
#include <kernel/lib/panic.h>
#include <kernel/lib/stdlib.h>

static void * _kmalloc(struct mem_block * block, unsigned int size);
static void _splitBlock(struct mem_block * block, unsigned int size);
static void _kdefrag();

/*
Agrandi le tas

Va chercher n pages en m�moire
Les ajoute ces pages � l'espace d'adressage du noyau
Les nouveaux bloques sont initialis�s
*/
struct mem_block * ksbrk(int n)
{
	// +1 car on prend en compte la taille du bloque point� par g_heap
	if ((g_last_heap_block + ((n + 1) * DEFAULT_BLOCK_SIZE) + (BLOCK_HEADER_SIZE * 2)) >= HEAP_LIMIT_ADDR)
	{
		panic("HEAP_LIMIT\n");
	}
	else
	{
		struct mem_block * new_block_v_addr = g_heap;
		int i = 0;

		if (new_block_v_addr->size != 0)
			new_block_v_addr = g_last_heap_block + g_last_heap_block->size + BLOCK_HEADER_SIZE;

		g_last_heap_block = new_block_v_addr;

		for (; i < n; i++)
		{
			u32 * new_page = (u32*)get_free_page();

			if (new_page == NULL)
				panic(MEMORY_FULL);

			pd_add_page(new_page, (u32)(new_block_v_addr + (i * DEFAULT_BLOCK_SIZE)));
		}

		new_block_v_addr->size = n * DEFAULT_BLOCK_SIZE;
		new_block_v_addr->state = BLOCK_FREE;
		mmset(&(new_block_v_addr->data), new_block_v_addr->size, 0);

		return new_block_v_addr;
	}
}

void * kmalloc(int size)
{
	void * res = NULL;
	// size est la taille en octets d�sir�e
	// On travaille sur la taille totale d'un bloque, donc en ajoutant la taille du header
	res = _kmalloc(g_heap, size);
	_kdefrag();
	return res;
}

void kfree(void * ptr)
{
	struct mem_block * block = (struct mem_block*)((int)ptr - sizeof(int));
	block->state = BLOCK_FREE;
	mmset(&(block->data), block->size, 0);
}

static void * _kmalloc(struct mem_block * block, int size)
{
	void * res_ptr = NULL;

	while (block <= g_last_heap_block && res_ptr == NULL)
	{
		if (block->state == BLOCK_USED || size > block->size)
		{
			block = block + block->size + BLOCK_HEADER_SIZE;
			continue;
		}

		if ((block->size - (size + (int)(BLOCK_HEADER_SIZE))) >= (int)(BLOCK_HEADER_SIZE + MINIMAL_BLOCK_SIZE))
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
	struct mem_block * second_block = block + size + BLOCK_HEADER_SIZE;
	second_block->size = block->size - size - BLOCK_HEADER_SIZE;
	second_block->state = BLOCK_FREE;

	if (block == g_last_heap_block)
		g_last_heap_block = second_block;

	block->size = size;
}

static void _kdefrag()
{
	struct mem_block * block = g_heap;

	while (block < g_last_heap_block)
	{
		struct mem_block * next = block + block->size + BLOCK_HEADER_SIZE;

		if (block->state == BLOCK_FREE && next->state == BLOCK_FREE)
		{
			block->size += next->size + BLOCK_HEADER_SIZE;
			mmset(&(block->data), 0, block->size);
			if (next == g_last_heap_block)
				g_last_heap_block = block;
		}
		else
		{
			block += block->size + BLOCK_HEADER_SIZE;
		}
	}
}