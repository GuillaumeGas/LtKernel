#include <kernel/lib/types.h>
#include <kernel/lib/stdlib.h>
#include <kernel/lib/stdio.h>
#include <kernel/lib/kmalloc.h>
#include <kernel/lib/panic.h>
#include <kernel/kernel.h>
#include <kernel/multiboot.h>
#include <kernel/init/heap.h>
#include <kernel/user/process_manager.h>

#include "vmm.h"

extern void _init_vmm(PageDirectoryEntry * pd0_addr);
static void InitKernelPageDirectoryAndPageTables();

// Représente l'ensemble des pages disponibles ou non
static u8 mem_bitmap[MEM_BITMAP_SIZE];

#define set_page_used(page) mem_bitmap[((u32)page) / 8] |= (1 << (((u32)page) % 8))
#define set_page_unused(addr) mem_bitmap[((u32)addr / PAGE_SIZE) / 8] &= ~(1 << (((u32)addr / PAGE_SIZE) % 8))

/*
	Initialisation simple de la mémoire virtuelle :
	 - Identity mapping pour le noyau
*/
void InitVmm()
{
	int page = 0;

	// On calcul la dernière page
	int lastPage = (g_mbi.high_mem * 1024) / PAGE_SIZE;

	// Initialisation du bitmap à 0 : toutes les pages sont dispo
	mmset(mem_bitmap, 0, MEM_BITMAP_SIZE);

	// On bloque les pages inexistantes (si on a moins de 4Go de RAM en réalité)
	for (page = lastPage / 8; page < RAM_MAXPAGE / 8; page++)
		mem_bitmap[page] = 0xFF;

	// Réserve les pages du noyau
	for (page = PAGE(0); page < PAGE(g_kernelInfo.kernelLimit_p); page++)
		set_page_used(page);

	// On met à 0 le répertoire de pages
	CleanAllPageDirectoryAndPageTables(g_kernelInfo.pageDirectory_p.pd_entry, g_kernelInfo.pageTables_p);

	// Initialise tous le répertoire de pages du noyau ainsi que toutes les tables de pages
	InitKernelPageDirectoryAndPageTables();

	// Identity mapping (v_addr == p_addr pour le noyau, donc entre 0x0 et 0x800000)
	{
		PageTableEntry * kernelFirstPt = g_kernelInfo.pageTables_p;
		PageTableEntry * kernelSecondPt = (PageTableEntry *)((unsigned int)kernelFirstPt + PAGE_SIZE);

		SetPageDirectoryEntry(g_kernelInfo.pageDirectory_p.pd_entry, (u32)kernelFirstPt, PAGE_PRESENT | PAGE_WRITEABLE);
		SetPageDirectoryEntry(&(g_kernelInfo.pageDirectory_p.pd_entry[1]), (u32)kernelSecondPt, PAGE_PRESENT | PAGE_WRITEABLE);

		unsigned int ptIndex = 0;
		for (page = PAGE(0); page < PAGE(g_kernelInfo.kernelLimit_p); page++)
		{
			SetPageTableEntry(&(kernelFirstPt[ptIndex]), ptIndex * PAGE_SIZE, PAGE_PRESENT | PAGE_WRITEABLE);
			ptIndex++;
		}

		SetPageTableEntry(&(kernelFirstPt[1023]), (u32)kernelFirstPt, PAGE_PRESENT | PAGE_WRITEABLE);
		SetPageTableEntry(&(kernelSecondPt[1023]), (u32)kernelSecondPt, PAGE_PRESENT | PAGE_WRITEABLE);
	}

	_init_vmm(g_kernelInfo.pageDirectory_p.pd_entry);

	init_heap();
	init_page_heap();
	
	g_kernelInfo.pageDirectory_p.page_table_list = ListCreate();
}

// Initialise tous le répertoire de pages du noyau ainsi que toutes les tables de pages
void InitKernelPageDirectoryAndPageTables()
{
	PageDirectoryEntry * pageDirectory = g_kernelInfo.pageDirectory_p.pd_entry;
	PageTableEntry * pageTable = g_kernelInfo.pageTables_p;
	unsigned int pdIndex = 0;

	for (; pdIndex < NB_PAGES_TABLE_PER_DIRECTORY; pdIndex++)
	{
		SetPageDirectoryEntry(&(pageDirectory[pdIndex]), (u32)pageTable, PAGE_PRESENT | PAGE_WRITEABLE);

		unsigned int j = 0;
		for (; j < NB_PAGES_PER_TABLE; j++)
		{
			SetPageTableEntry(&(pageTable[j]), 0, PAGE_PRESENT | PAGE_WRITEABLE);
		}

		SetPageTableEntry(&(pageTable[1023]), (u32)pageTable, PAGE_PRESENT | PAGE_WRITEABLE);

		pageTable = (PageTableEntry *)((unsigned int)pageTable + PAGE_SIZE);
	}

	SetPageDirectoryEntry(&(pageDirectory[1023]), (u32)pageDirectory, PAGE_PRESENT | PAGE_WRITEABLE);
}

void VmmCleanCallback()
{
    CleanPageHeap();
    // TODO : il faudra sans doute free le contenu de chaque élément à l'avenir !
    ListDestroy(g_kernelInfo.pageDirectory_p.page_table_list);
}

void CleanAllPageDirectoryAndPageTables(PageDirectoryEntry * pageDirectoryEntry, PageTableEntry * pageTableEntry)
{
	CleanPageDirectory(pageDirectoryEntry);

	unsigned int pdIndex = 0;
	for (; pdIndex < NB_PAGES_TABLE_PER_DIRECTORY; pdIndex++)
	{
		CleanPageTable(pageTableEntry);
		pageTableEntry = (PageTableEntry *)((unsigned int)pageTableEntry + PAGE_SIZE);
	}
}

void CleanPageDirectory(PageDirectoryEntry * pageDirectoryEntry)
{
	unsigned int index = 0;

	for (; index < NB_PAGES_TABLE_PER_DIRECTORY; index++)
		SetPageDirectoryEntry(&(pageDirectoryEntry[index]), 0, PAGE_EMPTY);
}

void CleanPageTable(PageTableEntry * pageTableEntry)
{
	unsigned int index = 0;

	for (; index < NB_PAGES_PER_TABLE; index++)
		SetPageTableEntry(&(pageTableEntry[index]), 0, PAGE_EMPTY);
}

/*
	Initialise un répertoire de pages

	Si le flag PAGE_SIZE_4KO et 4MO sont à 1, 4MO l'emporte
*/
void SetPageDirectoryEntry(PageDirectoryEntry * pd, u32 pt_addr, PAGE_FLAG flags)
{
	if (flags == PAGE_EMPTY)
		mmset((u8 *)pd, 0, sizeof(PageDirectoryEntry));

	u32 * addr = (u32 *)pd;
	*addr = pt_addr;
	pd->present = FlagOn(flags, PAGE_PRESENT);
	pd->writable = FlagOn(flags, PAGE_WRITEABLE);
	pd->non_privileged_access = FlagOn(flags, PAGE_NON_PRIVILEGED_ACCESS);
}

/*
	Fait appel à la fonction d'initialisation d'un répertoire de pages

	Permet de définir le champ de gestion du cache et le champ librement utilisable
*/
void SetPageDirectoryEntryEx(PageDirectoryEntry * pd, u32 pt_addr, PAGE_FLAG flags, u8 global, u8 avail)
{
	SetPageDirectoryEntry(pd, pt_addr, flags);
	pd->avail = avail;
	pd->global = global;
}

/*
	Initialise une table de pages

	Fait appel à la fonction d'initialisation d'un répertoire de pages car leur structure
	 est identique.
*/
void SetPageTableEntry(PageTableEntry * pt, u32 page_addr, PAGE_FLAG flags)
{
	if (flags == PAGE_EMPTY)
		mmset((u8 *)pt, 0, sizeof(PageTableEntry));

	u32 * addr = (u32 *)pt;
	*addr = page_addr;
	pt->present = FlagOn(flags, PAGE_PRESENT);
	pt->writable = FlagOn(flags, PAGE_WRITEABLE);
	pt->non_privileged_access = FlagOn(flags, PAGE_NON_PRIVILEGED_ACCESS);
}

/*
	Initialise une table de pages

	Fait appel à la fonction d'initialisation d'un répertoire de pages car leur structure
	 est identique.

	Permet de définir le champ de gestion du cache et le champ librement utilisable
*/
void SetPageTableEntryEx(PageTableEntry * pt, u32 page_addr, PAGE_FLAG flags, u8 global, u8 avail)
{
	SetPageTableEntry(pt, page_addr, flags);
	pt->avail = avail;
	pt->global = global;
}

/*
	Va chercher dans le bitmap une page de disponible et renvoie son adresse physique
*/
void * GetFreePage()
{
	int byte, bit;
	for (byte = 0; byte < MEM_BITMAP_SIZE; byte++)
	{ 
		if (mem_bitmap[byte] != 0xFF)
		{
			for (bit = 0; bit < 8; bit++)
			{
				u8 b = mem_bitmap[byte];
				if (!(b & (1 << bit)))
				{
					u32 page = 8 * byte + bit;
					set_page_used(page);
					return (void*)(page * PAGE_SIZE);
				}
			}
		}
	}
	return (void*)(-1);
}

void ReleasePage(void * p_addr)
{
	set_page_unused(p_addr);
}


/*
	Récupère une adresse physique à partir d'une adresse virtuelle
*/
void * GetPhysicalAddress(void * v_addr)
{
	u32 * pde = NULL;
	u32 * pte = NULL;

	// Rappel d'une adresse v : [ offset dir (10 bits) | offset table (10 bits) | offset p (12 bits) ]

	// Les 10 premiers bits de l'adresse v représente un offset dans le répertoire, on les récupère avec le masque 0xFFC00000 et on décale
	// On fait un OU avec 0xFFFFF000 pour que les premiers bits soient à 1, le trick nous permet de retomber sur l'adresse du répertoire qu'on ne connait pas
	//  (la dernière entrée pointe sur le répertoire lui-même, puis avec les bits suivant aussi à 1, on se retrouve sur la dernière entrée du répertoire 
	//   qui pointe à nouveau sur le début du répertoire). Là, il ajoute l'offset qu'on a foutu à la fin de l'adresse.)
	// Ceci nous permet juste de vérifier si la table qui nous intéresse est en mémoire ou non.
	pde = (u32 *)(0xFFFFF000 | PD_OFFSET((u32)v_addr));

	if ((*pde & PAGE_PRESENT))
	{
		// On récupère maintenant un pointeur sur l'entrée de la table afin de savoir si la page en question est en mémoire.
		// On a encore le trick (0xFFC00000) qui nous permet de pointer sur le début du répertoire dans un premier temps
		// Ensuite, au lieu d'utiliser les 10 prochains bits comme un offset sur la table de page, ce sera sur le répertoire afin de pointer sur la bonne table
		// Les derniers bits, au lieu de représenter un offset physique, représente l'offset dans la table de page, afin de pointer sur l'entrée qui nous intéresse
		// et vérifier qu'elle est bien en mémoire (on a donc les deux dernières info grace au masque ainsi qu'au décallage de 10 bits (ça ne devrait pas être 12 d'ailleurs ??)
		pte = (u32 *)(0xFFC00000 | (((u32)v_addr & 0xFFFFF000) >> 10));
		if ((*pte & PAGE_PRESENT))
			// On termine en faisant ce que fait le CPU, prendre ce qui est pointé par l'entrée de la table et ajouter l'offset (les 12 derniers bits de l'adresse v).
			return (void *)((*pte & 0xFFFFF000) + ((((u32)v_addr)) & 0x00000FFF));
	}

	return (void *)NULL;
}

/*
	Met à jour l'espace d'adressage du noyau
*/
void AddPageToKernelPageDirectory(u8 * v_addr, u8 * p_addr, PAGE_FLAG flags)
{
	u32 * pde = NULL;
	u32 * pte = NULL;

	if (v_addr > (u8 *)USER_TASK_V_ADDR)
	{
		kprint("ERROR: pd0_add_page(): %p is not in kernel space !\n", v_addr);
		asm("hlt");
		return;
	}

	// On vérifie que la page est bien présente (voir get_p_addr pour mieux comprendre l'algo)
	pde = (u32 *)(0xFFFFF000 | PD_OFFSET((u32)v_addr));

	if (!FlagOn(*pde, PAGE_PRESENT))
	{
		panic(PAGE_TABLE_NOTE_FOUND);
	}

	// Modification de l'entrée dans la table de pages
	pte = (u32 *)(0xFFC00000 | (((u32)v_addr & 0xFFFFF000) >> 10));

	SetPageTableEntry((PageTableEntry *)pte, (u32)p_addr, PAGE_PRESENT | PAGE_WRITEABLE);
}

/*
	Met à jour le répertoire de pages passé en paramètre

	[!] L'adresse du répertoire de pages doit avoir été renseigné dans le registre cr3 au préalable !
*/
void AddPageToPageDirectory(u8 * v_addr, u8 * p_addr, PAGE_FLAG flags, PageDirectory pd)
{
	u32 * pde = NULL; // adresse virtuelle de l'entrée du répertoire de pages
	u32 * pte = NULL; // adresse virtuelle de l'entrée de la table de pages
	u32 * pt = NULL;  // adresse virtuelle de la table de pages

	// On vérifie que la page est bien présente (voir get_p_addr pour mieux comprendre l'algo)
	pde = (u32 *)(0xFFFFF000 | PD_OFFSET((u32)v_addr));

	if (!FlagOn(*pde, PAGE_PRESENT))
	{
		Page new_page = PageAlloc();
		pt = (u32 *)new_page.v_addr;

		// On ajoute la nouvelle page au répertoire de pages
		SetPageDirectoryEntry((PageDirectoryEntry *)pde, (u32)new_page.p_addr, PAGE_PRESENT | PAGE_WRITEABLE | flags);

		CleanPageTable((PageTableEntry *)pt);

		// On ajoute la nouvelle page à la liste de pages associée à ce répertoire (pour plus facilement libérer la mémoire après)
		ListPush(pd.page_table_list, new_page.v_addr);
	}

    pte = (u32 *)(0xFFC00000 | (((u32)v_addr & 0xFFFFF000) >> 10));

	SetPageTableEntry((PageTableEntry *)pte, (u32)p_addr, (PAGE_PRESENT | PAGE_WRITEABLE | flags));
}

static int testValue = 0;
PageDirectory CreateProcessPageDirectory()
{
	Page pd_page = PageAlloc();
	PageDirectory pd = { 0 };
	PageDirectoryEntry * kernelPdEntry = (PageDirectoryEntry *)g_kernelInfo.pageDirectory_p.pd_entry;
	PageDirectoryEntry * pd_entry = (PageDirectoryEntry *)pd_page.v_addr;
	unsigned int i = 0;
	
	//kprint("Task %d, v = %x, p : %x\n", testValue++, pd_page.v_addr, pd_page.p_addr);

	// On veut que le premier Go de mémoire virtuelle soit pour le noyau : 1024 / 4 = 256 (1024 = nombre d'entrées dans un répertoire de pages)
	// On vérifie:  256 * 1024 * 4096 = 1Go
	for (; i < 256; i++)
		pd_entry[i] = kernelPdEntry[i];

	for (i = 256; i < NB_PAGES_TABLE_PER_DIRECTORY; i++)
		SetPageDirectoryEntry(&(pd_entry[i]), 0, PAGE_PRESENT | PAGE_WRITEABLE | PAGE_NON_PRIVILEGED_ACCESS);

	SetPageDirectoryEntry(&(pd_entry[1023]), (u32)pd_page.p_addr, PAGE_PRESENT | PAGE_WRITEABLE);

	pd.pd_entry = (PageDirectoryEntry *)pd_page.p_addr;
	pd.page_table_list = ListCreate();

	return pd;
}