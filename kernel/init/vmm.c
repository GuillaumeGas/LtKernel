#define __MEMORY__
#include "vmm.h"

#include <kernel/lib/types.h>
#include <kernel/lib/stdlib.h>
#include <kernel/lib/stdio.h>
#include <kernel/lib/kmalloc.h>
#include <kernel/lib/panic.h>
#include <kernel/kernel.h>
#include <kernel/multiboot.h>

extern void _init_vmm(PageDirectoryEntry * pd0_addr);

// Repr�sente l'ensemble des pages disponibles ou non
static u8 mem_bitmap[MEM_BITMAP_SIZE];

#define set_page_used(page) mem_bitmap[((u32)page) / 8] |= (1 << (((u32)page) % 8))
#define set_page_unused(addr) mem_bitmap[((u32)addr / PAGE_SIZE) / 8] &= ~(1 << (((u32)addr / PAGE_SIZE) % 8))

/*
	Initialisation simple de la m�moire virtuelle :
	 - Identity mapping pour le noyau
*/
void init_vmm()
{
	int page = 0;

	// On calcul la derni�re page
	int lastPage = (g_mbi.high_mem * 1024) / PAGE_SIZE;

	// Initialisation du bitmap � 0 : toutes les pages sont dispo
	mmset(mem_bitmap, 0, RAM_MAXPAGE / 8);

	// On bloque les pages inexistantes (si on a moins de 4Go de RAM en r�alit�)
	for (page = lastPage / 8; page < RAM_MAXPAGE / 8; page++)
		mem_bitmap[page] = 0xFF;

	// R�serve les pages du noyau
	for (page = PAGE(0); page < PAGE(g_kernelInfo.kernelLimit_p); page++)
		set_page_used(page);

	//init_heap();
	//init_page_heap();

	// On met � 0 le r�pertoire de pages
	init_clean_pages_directory(g_kernelInfo.pageDirectory_p.pd_entry);

	// Identity mapping
	{
		u32 index = 0;
		PageTableEntry * kernelPt = g_kernelInfo.pageTables_p;

		set_page_directory_entry(g_kernelInfo.pageDirectory_p.pd_entry, (u32)kernelPt, IN_MEMORY | WRITEABLE);

		for (page = PAGE(0); page < PAGE(g_kernelInfo.kernelLimit_p); page++)
		{
			set_page_table_entry(&(kernelPt[index]), index * PAGE_SIZE, IN_MEMORY | WRITEABLE);
			index++;
		}
	}

	// Trick pour acc�der au contenu du r�pertoire et des tables de pages : la derni�re entr�e pointe sur l'adresse du r�pertoire.
	set_page_directory_entry(&(g_kernelInfo.pageDirectory_p.pd_entry[1023]), (u32)g_kernelInfo.pageDirectory_p.pd_entry, IN_MEMORY | WRITEABLE);

	_init_vmm(g_kernelInfo.pageDirectory_p.pd_entry);
}

void init_clean_pages_directory(PageDirectoryEntry * first_pd)
{
	unsigned int index = 0;

	for (; index < NB_PAGES_TABLE_PER_DIRECTORY; index++)
		set_page_directory_entry(&(first_pd[index]), 0, EMPTY);
}

void init_clean_pages_table(PageTableEntry * first_pt)
{
	unsigned int index = 0;

	for (; index < NB_PAGES_PER_TABLE; index++)
		set_page_table_entry(&(first_pt[index]), 0, EMPTY);
}

/*
	Initialise un r�pertoire de pages

	Si le flag PAGE_SIZE_4KO et 4MO sont � 1, 4MO l'emporte
*/
void set_page_directory_entry(PageDirectoryEntry * pd, u32 pt_addr, PD_FLAG flags)
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
	Fait appel � la fonction d'initialisation d'un r�pertoire de pages

	Permet de d�finir le champ de gestion du cache et le champ librement utilisable
*/
void set_page_directory_entryEx(PageDirectoryEntry * pd, u32 pt_addr, PD_FLAG flags, u8 global, u8 avail)
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

	Fait appel � la fonction d'initialisation d'un r�pertoire de pages car leur structure
	 est identique.
*/
void set_page_table_entry(PageTableEntry * pt, u32 page_addr, PT_FLAG flags)
{
	if (!FlagOn(EMPTY, flags))
		pt->written = FlagOn(WRITTEN, flags);

	set_page_directory_entry((PageDirectoryEntry *)pt, page_addr, flags);
}

/*
	Initialise une table de pages

	Fait appel � la fonction d'initialisation d'un r�pertoire de pages car leur structure
	 est identique.

	Permet de d�finir le champ de gestion du cache et le champ librement utilisable
*/
void set_page_table_entryEx(PageTableEntry * pt, u32 page_addr, PT_FLAG flags, u8 global, u8 avail)
{
	if (!FlagOn(EMPTY, flags))
		pt->written = FlagOn(WRITTEN, flags);

	set_page_directory_entryEx((PageDirectoryEntry *)pt, page_addr, flags, global, avail);
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

void release_page(void * p_addr)
{
	set_page_unused(p_addr);
}


/*
	R�cup�re une adresse physique � partir d'une adresse virtuelle
*/
void * get_p_addr(void * v_addr)
{
	u32 * pde = NULL;
	u32 * pte = NULL;

	// Rappel d'une adresse v : [ offset dir (10 bits) | offset table (10 bits) | offset p (12 bits) ]

	// Les 10 premiers bits de l'adresse v repr�sente un offset dans le r�pertoire, on les r�cup�re avec le masque 0xFFC00000 et on d�cale
	// On fait un OU avec 0xFFFFF000 pour que les premiers bits soient � 1, le trick nous permet de retomber sur l'adresse du r�pertoire qu'on ne connait pas
	//  (la derni�re entr�e pointe sur le r�pertoire lui-m�me, puis avec les bits suivant aussi � 1, on se retrouve sur la derni�re entr�e du r�pertoire 
	//   qui pointe � nouveau sur le d�but du r�pertoire). L�, il ajoute l'offset qu'on a foutu � la fin de l'adresse.)
	// Ceci nous permet juste de v�rifier si la table qui nous int�resse est en m�moire ou non.
	pde = (u32 *)(0xFFFFF000 | (((u32)v_addr & 0xFFC00000) >> 20));

	if ((*pde & IN_MEMORY))
	{
		// On r�cup�re maintenant un pointeur sur l'entr�e de la table afin de savoir si la page en question est en m�moire.
		// On a encore le trick (0xFFC00000) qui nous permet de pointer sur le d�but du r�pertoire dans un premier temps
		// Ensuite, au lieu d'utiliser les 10 prochains bits comme un offset sur la table de page, ce sera sur le r�pertoire afin de pointer sur la bonne table
		// Les derniers bits, au lieu de repr�senter un offset physique, repr�sente l'offset dans la table de page, afin de pointer sur l'entr�e qui nous int�resse
		// et v�rifier qu'elle est bien en m�moire (on a donc les deux derni�res info grace au masque ainsi qu'au d�callage de 10 bits (�a ne devrait pas �tre 12 d'ailleurs ??)
		pte = (u32 *)(0xFFC00000 | (((u32)v_addr & 0xFFFFF000) >> 10));
		if ((*pte & IN_MEMORY))
			// On termine en faisant ce que fait le CPU, prendre ce qui est point� par l'entr�e de la table et ajouter l'offset (les 12 derniers bits de l'adresse v).
			return (void *)((*pte & 0xFFFFF000) + ((((u32)v_addr)) & 0x00000FFF));
	}

	return (void *)NULL;
}

/*
	Met � jour l'espace d'adressage du noyau
*/
void pd0_add_page(u8 * v_addr, u8 * p_addr, PT_FLAG flags)
{
	u32 * pde = NULL;
	u32 * pte = NULL;

	if (v_addr > (u8 *)USER_TASK_V_ADDR)
	{
		kprint("ERROR: pd0_add_page(): %p is not in kernel space !\n", v_addr);
		return;
	}

	// On v�rifie que la page est bien pr�sente (voir get_p_addr pour mieux comprendre l'algo)
	pde = (u32 *)(0xFFFFF000 | PD_OFFSET((u32)v_addr));
	if (!FlagOn(*pde, IN_MEMORY))
		panic(PAGE_TABLE_NOTE_FOUND);

	// Modification de l'entr�e dans la table de pages
	pte = (u32 *)(0xFFC00000 | (((u32)v_addr & 0xFFFFF000) >> 10));
	*pte = ((u32)p_addr) | (IN_MEMORY | WRITEABLE | flags);
}

/*
	Met � jour le r�pertoire de pages pass� en param�tre

	[!] L'adresse du r�pertoire de pages doit avoir �t� renseign� dans le registre cr3 au pr�alable !
*/
void pd_add_page(u8 * v_addr, u8 * p_addr, PT_FLAG flags, PageDirectory pd)
{
	u32 * pde = NULL; // adresse virtuelle de l'entr�e du r�pertoire de pages
	u32 * pte = NULL; // adresse virtuelle de l'entr�e de la table de pages
	u32 * pt = NULL;  // adresse virtuelle de la table de pages

	// On v�rifie que la page est bien pr�sente (voir get_p_addr pour mieux comprendre l'algo)
	pde = (u32 *)(0xFFFFF000 | PD_OFFSET((u32)v_addr));

	if (!FlagOn(*pde, IN_MEMORY))
	{
		Page new_page = page_alloc();
		pt = (u32 *)new_page.v_addr;

		// On ajoute la nouvelle page au r�pertoire de pages
		set_page_directory_entry((PageDirectoryEntry *)pde, (u32)new_page.p_addr, IN_MEMORY | WRITEABLE | flags);

		init_clean_pages_table((PageTableEntry *)pt);

		// On ajoute la nouvelle page � la liste de pages associ�e � ce r�pertoire (pour plus facilement lib�rer la m�moire apr�s)
		list_push(pd.page_table_list, new_page.v_addr);
	}

	pte = (u32 *) ((0xFFC00000 | (PD_OFFSET((u32)v_addr) << 10)));
	set_page_table_entry((PageTableEntry *)pte, (u32)p_addr, (IN_MEMORY | WRITEABLE | flags));
}

/*
Initialise le tas avec un un bloc (taille d'une page)
*/
void init_heap()
{
	g_heap = (MemBlock *)g_kernelInfo.heapBase_v;
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

	g_page_heap = (MemPageBlock *)kmalloc(sizeof(MemPageBlock));
	g_page_heap->available = BLOCK_FREE;
	g_page_heap->next = NULL;
	g_page_heap->v_page_addr = (u32 *)g_kernelInfo.pagesHeapBase_v;
	g_page_heap->prev = NULL;

	tmp = g_page_heap;

	while (tmp->v_page_addr < (u32 *)g_kernelInfo.pagesHeapLimit_v)
	{
		tmp->next = (MemPageBlock *)kmalloc(sizeof(MemPageBlock));

		prev = tmp;
		tmp = tmp->next;

		tmp->v_page_addr = prev->v_page_addr + PAGE_SIZE;
		tmp->available = BLOCK_FREE;
		tmp->prev = prev;
		tmp->next = NULL;
	}
}

PageDirectory create_process_pd()
{
	Page pd_page = page_alloc();
	PageDirectory pd = { 0 };
	PageDirectoryEntry * kernelPdEntry = (PageDirectoryEntry *)g_kernelInfo.pageDirectory_p.pd_entry;
	PageDirectoryEntry * pd_entry = (PageDirectoryEntry *)pd_page.p_addr;
	unsigned int i = 0;

	// On veut que le premier Go de m�moire virtuelle soit pour le noyau : 1024 / 4 = 256 (1024 = nombre d'entr�es dans un r�pertoire de pages)
	// On v�rifie:  256 * 1024 * 4096 = 1Go
	for (; i < 256; i++)
		pd_entry[i] = kernelPdEntry[i];

	for (i = 256; i < NB_PAGES_TABLE_PER_DIRECTORY; i++)
		set_page_directory_entry(&(pd_entry[i]), 0, EMPTY);

	set_page_directory_entry(&(pd_entry[1023]), (u32)pd_entry, IN_MEMORY | WRITEABLE);

	pd.pd_entry = pd_entry;
	pd.page_table_list = list_create();

	return pd;
}