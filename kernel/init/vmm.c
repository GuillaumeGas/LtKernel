#include <kernel/lib/types.h>
#include <kernel/lib/stdlib.h>
#include <kernel/lib/stdio.h>

#include "vmm.h"

static void set_page_directory_entry(struct page_directory_entry * pd, u32 pt_addr, PD_FLAG flags);
static void set_page_directory_entryEx(struct page_directory_entry * pd, u32 pt_addr, PD_FLAG flags, u8 global, u8 avail);
static void set_page_table_entry(struct page_table_entry * pt, u32 page_addr, PT_FLAG flags);
static void set_page_table_entryEx(struct page_table_entry * pt, u32 page_addr, PT_FLAG flags, u8 global, u8 avail);

extern void _init_vmm(struct page_directory_entry * pd0_addr);

void init_vmm()
{
	struct page_directory_entry * pd0 = (struct page_directory_entry*)PD0_ADDR;
	struct page_table_entry * pt0 = (struct page_table_entry*)PT0_ADDR;
	unsigned int index = 0;
	unsigned int kernel_base_addr = 0;

	set_page_directory_entry(pd0, (u32)pt0, IN_MEMORY | WRITEABLE);

	for (index = 1; index < NB_PAGES_TABLE_PER_DIRECTORY; index++)
		set_page_directory_entry(&(pd0[index]), 0, EMPTY);

	// pour l'instant on va tester sur 32 pages pour le noyau, ce qui est actuellement utilisé
	for (index = 0; index < NB_PAGES_PER_TABLE; index++)
		set_page_table_entry(&(pt0[index]), kernel_base_addr + (index * PAGE_SIZE), IN_MEMORY | WRITEABLE);

	_init_vmm(pd0);
}

/*
	Initialise un répertoire de pages

	Si le flag PAGE_SIZE_4KO et 4MO sont à 1, 4MO l'emporte
*/
static void set_page_directory_entry(struct page_directory_entry * pd, u32 pt_addr, PD_FLAG flags)
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
static void set_page_directory_entryEx(struct page_directory_entry * pd, u32 pt_addr, PD_FLAG flags, u8 global, u8 avail)
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
static void set_page_table_entry(struct page_table_entry * pt, u32 page_addr, PT_FLAG flags)
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
static void set_page_table_entryEx(struct page_table_entry * pt, u32 page_addr, PT_FLAG flags, u8 global, u8 avail)
{
	if (!FlagOn(EMPTY, flags))
		pt->written = FlagOn(WRITTEN, flags);

	set_page_directory_entryEx((struct page_directory_entry*)pt, page_addr, flags, global, avail);
}