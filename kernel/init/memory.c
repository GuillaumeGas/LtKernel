#include <kernel/lib/types.h>
#include <kernel/lib/stdlib.h>
#include <kernel/lib/stdio.h>
#include <kernel/lib/kmalloc.h>

#define __MEMORY__
#include "memory.h"

extern void _init_vmm(struct page_directory_entry * pd0_addr);

// Représente l'ensemble des pages disponibles ou non
static u8 mem_bitmap[MEM_BITMAP_SIZE];

#define set_page_used(page) mem_bitmap[((u32)page) / 8] |= (1 << (((u32)page) % 8))
#define set_page_unused(addr) mem_bitmap[((u32)addr / PAGE_SIZE) / 8] &= ~(1 << (((u32)addr / PAGE_SIZE) % 8))
#define PAGE(addr) (addr) >> 12

/*
	Initialisation simple de la mémoire virtuelle :
	 - 4Mo allouables (1 entrée dans le répertoire de pages -- 1024 entrées dans la table des pages)
	     1 * 1024 * 4ko = 4Mo
     - addr virtuelle 0 == addr physique 0, etc...
*/
void init_vmm()
{
	unsigned int index = 0;
	int page = 0;

	// Initialisation du bitmap à 0 : toutes les pages sont dispo
	mmset(mem_bitmap, 0, RAM_MAXPAGE / 8);

	// Réserve les pages du noyau
	for (page = PAGE(0); page < PAGE(KERNEL_P_LIMIT_ADDR); page++)
		set_page_used(page);

	init_heap();
	init_page_heap();

	// L'adresse du répertoire de pages du noyau est fixe
	g_kernel_pd = (struct page_directory_entry*)PD0_ADDR;
	// On va allouer une page pour y stocker une table de pages
	g_kernel_pt = (struct page_table_entry*)PT0_ADDR;

	// On met à 0 le répertoire de pages
	init_pages_directory(g_kernel_pd);

	// Le noyau aura accès à toute la mémoire !!!!!!!!!!
	{
		struct page_table_entry * tmp = g_kernel_pt;
		int i = 0;
		
		for (; i < 1024; i++)
		{
			struct page_table_entry * current_pt = g_kernel_pt + (i * PAGE_SIZE);

			// Initialisation de l'entrée du répertoire du page concernant le noyau
			set_page_directory_entry(&(g_kernel_pd[i]), (u32)current_pt, IN_MEMORY | WRITEABLE);

			// Mapping simple : l'adresse v addr == p addr
			for (index = 0; index < NB_PAGES_PER_TABLE; index++)
				set_page_table_entry(&(current_pt[index]), ((index * PAGE_SIZE) + (i * NB_PAGES_PER_TABLE * PAGE_SIZE)), IN_MEMORY | WRITEABLE);
		}
	}

	_init_vmm(g_kernel_pd);
}

void init_pages_directory(struct page_directory_entry * first_pd)
{
	unsigned int index = 0;

	for (; index < NB_PAGES_TABLE_PER_DIRECTORY; index++)
		set_page_directory_entry(&(first_pd[index]), 0, EMPTY);
}

/*
	Initialise un répertoire de pages

	Si le flag PAGE_SIZE_4KO et 4MO sont à 1, 4MO l'emporte
*/
void set_page_directory_entry(struct page_directory_entry * pd, u32 pt_addr, PD_FLAG flags)
{
	u32 * pd_addr = (u32*)pd;
	(*pd_addr) = EMPTY_PAGE_TABLE;

	if (FlagOn(EMPTY, flags))
		return;
	else
		(*pd_addr) = pt_addr;

	pd->in_memory = FlagOn(IN_MEMORY, flags);
	pd->writable = FlagOn(WRITEABLE, flags);
	pd->non_privileged_access = FlagOn(NON_PRIVILEGED_ACCESS, flags);
	pd->pwt = FlagOn(PWT, flags);
	pd->pcd = FlagOn(PCD, flags);
	pd->accessed = FlagOn(ACCESSED, flags);
	pd->page_size = FlagOn(PAGE_SIZE_4MO, flags);
}

/*
	Fait appel à la fonction d'initialisation d'un répertoire de pages

	Permet de définir le champ de gestion du cache et le champ librement utilisable
*/
void set_page_directory_entryEx(struct page_directory_entry * pd, u32 pt_addr, PD_FLAG flags, u8 global, u8 avail)
{
	if (!FlagOn(EMPTY, flags))
	{
		pd->global = global;
		pd->avail = avail;
	}

	set_page_directory_entry(pd, pt_addr, flags);
}

/*
	Initialise une table de pages

	Fait appel à la fonction d'initialisation d'un répertoire de pages car leur structure
	 est identique.
*/
void set_page_table_entry(struct page_table_entry * pt, u32 page_addr, PT_FLAG flags)
{
	if (!FlagOn(EMPTY, flags))
		pt->written = FlagOn(WRITTEN, flags);

	set_page_directory_entry((struct page_directory_entry*)pt, page_addr, flags);
}

/*
	Initialise une table de pages

	Fait appel à la fonction d'initialisation d'un répertoire de pages car leur structure
	 est identique.

	Permet de définir le champ de gestion du cache et le champ librement utilisable
*/
void set_page_table_entryEx(struct page_table_entry * pt, u32 page_addr, PT_FLAG flags, u8 global, u8 avail)
{
	if (!FlagOn(EMPTY, flags))
		pt->written = FlagOn(WRITTEN, flags);

	set_page_directory_entryEx((struct page_directory_entry*)pt, page_addr, flags, global, avail);
}

/*
	Va chercher dans le bitmap une page de disponible et renvoie son adresse physique
*/
void * get_free_page()
{
	unsigned int index = 0;

	for (; index < MEM_BITMAP_SIZE; index++)
	{
		if (mem_bitmap[index] != 0xFF)
		{
			u8 byte = mem_bitmap[index];
			unsigned int offset = 0;

			for (; offset < sizeof(u8); offset++)
			{
				byte >>= offset;

				if ((byte & 1) == 0)
				{
					int page = 8 * index + offset;
					set_page_used(page);
					return (void*)(page * PAGE_SIZE);
				}
			}
		}
	}

	return (void*)(-1);
}

/*
Initialise le tas avec un un bloc (taille d'une page)
*/
void init_heap()
{
	g_heap = (struct mem_block*)HEAP_BASE_ADDR;
	g_last_heap_block = g_heap;

	ksbrk(1);
}

/*
	Initialise le tas de pages
*/
void init_page_heap()
{
	struct mem_pblock * tmp = NULL;
	struct mem_pblock * prev = NULL;

	g_page_heap = (struct mem_pblock *)kmalloc(sizeof(struct mem_pblock));
	g_page_heap->available = BLOCK_FREE;
	g_page_heap->next = NULL;
	g_page_heap->page_addr = PAGE_HEAP_BASE_ADDR;
	g_page_heap->prev = NULL;

	tmp = g_page_heap;

	while (tmp->page_addr < PAGE_HEAP_LIMIT_ADDR)
	{
		tmp->next = (struct mem_pblock *)kmalloc(sizeof(struct mem_pblock));

		prev = tmp;
		tmp = tmp->next;

		tmp->page_addr = prev->page_addr + PAGE_SIZE;
		tmp->available = BLOCK_FREE;
		tmp->prev = prev;
		tmp->next = NULL;
	}
}