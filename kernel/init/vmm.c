#include <kernel/lib/types.h>
#include <kernel/lib/stdlib.h>
#include <kernel/lib/stdio.h>
#include <kernel/lib/kmalloc.h>
#include <kernel/lib/panic.h>
#include <kernel/kernel.h>
#include <kernel/multiboot.h>
#include <kernel/init/heap.h>
#include <kernel/user/process_manager.h>
#include <kernel/debug/debug.h>

#include <kernel/logger.h>
#define KLOG(LOG_LEVEL, format, ...) KLOGGER("VMM", LOG_LEVEL, format, ##__VA_ARGS__)

#include "vmm.h"

extern void _init_vmm(PageDirectoryEntry * pd0_addr);
static void InitKernelPageDirectoryAndPageTables();
static void SetIdentityMapping();

// Représente l'ensemble des pages disponibles ou non
static u8 memBitmap[MEM_BITMAP_SIZE];

static PageDirectoryEntry * s_SavedPageDirectoryEntry = NULL;

#define set_page_used(page) memBitmap[((u32)page) / 8] |= (1 << (((u32)page) % 8))
#define set_page_unused(addr) memBitmap[((u32)addr / PAGE_SIZE) / 8] &= ~(1 << (((u32)addr / PAGE_SIZE) % 8))

/*
	Initialisation simple de la mémoire virtuelle :
	 - Identity mapping pour le noyau
*/
void InitVmm()
{
	int page = 0;

	// On calcul la dernière page
	int lastPage = (gMbi.high_mem * 1024) / PAGE_SIZE;

	// Initialisation du bitmap à 0 : toutes les pages sont dispo
	MmSet(memBitmap, 0, MEM_BITMAP_SIZE);

	// On bloque les pages inexistantes (si on a moins de 4Go de RAM en réalité)
	for (page = lastPage / 8; page < RAM_MAXPAGE / 8; page++)
		memBitmap[page] = 0xFF;

	// Réserve les pages du noyau
	for (page = PAGE(0); page < PAGE(gKernelInfo.pKernelLimit); page++)
		set_page_used(page);

	// On met à 0 le répertoire de pages
	CleanAllPageDirectoryAndPageTables(gKernelInfo.pPageDirectory.pdEntry, gKernelInfo.pPageTables);

	// Initialise tous le répertoire de pages du noyau ainsi que toutes les tables de pages
	InitKernelPageDirectoryAndPageTables();

	// Identity mapping (v_addr == p_addr pour le noyau, donc entre 0x0 et 0x800000)
    SetIdentityMapping();

	_init_vmm(gKernelInfo.pPageDirectory.pdEntry);

	HeapInit();
	PageHeapInit();
	
	gKernelInfo.pPageDirectory.pageTableList = ListCreate();
}

// Initialise tous le répertoire de pages du noyau ainsi que toutes les tables de pages
void InitKernelPageDirectoryAndPageTables()
{
	PageDirectoryEntry * pageDirectory = gKernelInfo.pPageDirectory.pdEntry;
	PageTableEntry * pageTable = gKernelInfo.pPageTables;
	unsigned int pdIndex = 0;

	for (; pdIndex < NB_PAGES_TABLE_PER_KERNEL_DIRECTORY; pdIndex++)
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

// Identity mapping (v_addr == p_addr pour le noyau, donc entre 0x0 et 0x800000)
void SetIdentityMapping()
{
    PageTableEntry * kernelFirstPt = gKernelInfo.pPageTables;
    PageTableEntry * kernelSecondPt = (PageTableEntry *)((unsigned int)kernelFirstPt + PAGE_SIZE);
    unsigned int page = 0;

    SetPageDirectoryEntry(gKernelInfo.pPageDirectory.pdEntry, (u32)kernelFirstPt, PAGE_PRESENT | PAGE_WRITEABLE);
    SetPageDirectoryEntry(&(gKernelInfo.pPageDirectory.pdEntry[1]), (u32)kernelSecondPt, PAGE_PRESENT | PAGE_WRITEABLE);

    unsigned int ptIndex = 0;
    for (page = PAGE(0); page < PAGE(gKernelInfo.pKernelLimit); page++)
    {
        SetPageTableEntry(&(kernelFirstPt[ptIndex]), ptIndex * PAGE_SIZE, PAGE_PRESENT | PAGE_WRITEABLE);
        ptIndex++;
    }

    SetPageTableEntry(&(kernelFirstPt[1023]), (u32)kernelFirstPt, PAGE_PRESENT | PAGE_WRITEABLE);
    SetPageTableEntry(&(kernelSecondPt[1023]), (u32)kernelSecondPt, PAGE_PRESENT | PAGE_WRITEABLE);
}

void VmmCleanCallback()
{
    CleanPageHeap();
    // TODO : il faudra sans doute free le contenu de chaque élément à l'avenir !
    ListDestroy(gKernelInfo.pPageDirectory.pageTableList);
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
*/
void SetPageDirectoryEntry(PageDirectoryEntry * pd, u32 ptAddr, PAGE_FLAG flags)
{
	if (flags == PAGE_EMPTY)
		MmSet((u8 *)pd, 0, sizeof(PageDirectoryEntry));

	u32 * addr = (u32 *)pd;
	*addr = ptAddr;
	pd->present = FlagOn(flags, PAGE_PRESENT);
	pd->writable = FlagOn(flags, PAGE_WRITEABLE);
	pd->nonPrivilegedAccess = FlagOn(flags, PAGE_NON_PRIVILEGED_ACCESS);
	pd->pwt = FlagOn(flags, PAGE_PWT);
	pd->pcd = FlagOn(flags, PAGE_PCD);
	pd->accessed = FlagOn(flags, PAGE_ACCESSED);
	pd->pageSize = FlagOn(flags, PAGE_SIZE_4MO);
}

/*
	Fait appel à la fonction d'initialisation d'un répertoire de pages

	Permet de définir le champ de geENABLE_IRQon du cache et le champ librement utilisable
*/
void SetPageDirectoryEntryEx(PageDirectoryEntry * pd, u32 ptAddr, PAGE_FLAG flags, u8 global, u8 avail)
{
	SetPageDirectoryEntry(pd, ptAddr, flags);
	pd->avail = avail;
	pd->global = global;
}

/*
	Initialise une table de pages

	Fait appel à la fonction d'initialisation d'un répertoire de pages car leur structure
	 est identique.
*/
void SetPageTableEntry(PageTableEntry * pt, u32 pageAddr, PAGE_FLAG flags)
{
	if (flags == PAGE_EMPTY)
		MmSet((u8 *)pt, 0, sizeof(PageTableEntry));

	u32 * addr = (u32 *)pt;
	*addr = pageAddr;
	pt->present = FlagOn(flags, PAGE_PRESENT);
	pt->writable = FlagOn(flags, PAGE_WRITEABLE);
	pt->nonPrivilegedAccess = FlagOn(flags, PAGE_NON_PRIVILEGED_ACCESS);
	pt->pwt = FlagOn(flags, PAGE_PWT);
	pt->pcd = FlagOn(flags, PAGE_PCD);
	pt->accessed = FlagOn(flags, PAGE_ACCESSED);
	pt->written = FlagOn(flags, PAGE_WRITTEN);
}

/*
	Initialise une table de pages

	Fait appel à la fonction d'initialisation d'un répertoire de pages car leur structure
	 est identique.

	Permet de définir le champ de geENABLE_IRQon du cache et le champ librement utilisable
*/
void SetPageTableEntryEx(PageTableEntry * pt, u32 pageAddr, PAGE_FLAG flags, u8 global, u8 avail)
{
	SetPageTableEntry(pt, pageAddr, flags);
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
		if (memBitmap[byte] != 0xFF)
		{
			for (bit = 0; bit < 8; bit++)
			{
				u8 b = memBitmap[byte];
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

void ReleasePage(void * pAddr)
{
	set_page_unused(pAddr);
}


/*
	Récupère une adresse physique à partir d'une adresse virtuelle
*/
void * GetPhysicalAddress(void * vAddr)
{
	u32 * pde = NULL;
	u32 * pte = NULL;

	// Rappel d'une adresse v : [ offset dir (10 bits) | offset table (10 bits) | offset p (12 bits) ]

	// Les 10 premiers bits de l'adresse v représente un offset dans le répertoire, on les récupère avec le masque 0xFFC00000 et on décale
	// On fait un OU avec 0xFFFFF000 pour que les premiers bits soient à 1, le trick nous permet de retomber sur l'adresse du répertoire qu'on ne connait pas
	//  (la dernière entrée pointe sur le répertoire lui-même, puis avec les bits suivant aussi à 1, on se retrouve sur la dernière entrée du répertoire 
	//   qui pointe à nouveau sur le début du répertoire). Là, il ajoute l'offset qu'on a foutu à la fin de l'adresse.)
	// Ceci nous permet juste de vérifier si la table qui nous intéresse est en mémoire ou non.
	pde = (u32 *)(0xFFFFF000 | PD_OFFSET((u32)vAddr));

	if ((*pde & PAGE_PRESENT))
	{
		// On récupère maintenant un pointeur sur l'entrée de la table afin de savoir si la page en queENABLE_IRQon est en mémoire.
		// On a encore le trick (0xFFC00000) qui nous permet de pointer sur le début du répertoire dans un premier temps
		// Ensuite, au lieu d'utiliser les 10 prochains bits comme un offset sur la table de page, ce sera sur le répertoire afin de pointer sur la bonne table
		// Les derniers bits, au lieu de représenter un offset physique, représente l'offset dans la table de page, afin de pointer sur l'entrée qui nous intéresse
		// et vérifier qu'elle est bien en mémoire (on a donc les deux dernières info grace au masque ainsi qu'au décallage de 10 bits (ça ne devrait pas être 12 d'ailleurs ??)
		pte = (u32 *)(0xFFC00000 | (((u32)vAddr & 0xFFFFF000) >> 10));
		if ((*pte & PAGE_PRESENT))
			// On termine en faisant ce que fait le CPU, prendre ce qui est pointé par l'entrée de la table et ajouter l'offset (les 12 derniers bits de l'adresse v).
			return (void *)((*pte & 0xFFFFF000) + ((((u32)vAddr)) & 0x00000FFF));
	}

	return (void *)NULL;
}

/*
	Met à jour l'espace d'adressage du noyau
*/
void AddPageToKernelPageDirectory(u8 * vAddr, u8 * pAddr, PAGE_FLAG flags)
{
	u32 * pde = NULL;
	u32 * pte = NULL;

	if (vAddr > (u8 *)USER_TASK_V_ADDR)
	{
		KLOG(LOG_ERROR, "%p is not in kernel space !", vAddr);
		asm("hlt");
		return;
	}

	// On vérifie que la page est bien présente (voir get_p_addr pour mieux comprendre l'algo)
	pde = (u32 *)(0xFFFFF000 | PD_OFFSET((u32)vAddr));

	if (!FlagOn(*pde, PAGE_PRESENT))
	{
		KLOG(LOG_ERROR, "Page not found (0x%x)", pde);
		panic(PAGE_TABLE_NOTE_FOUND);
	}

	// Modification de l'entrée dans la table de pages
	pte = (u32 *)(0xFFC00000 | (((u32)vAddr & 0xFFFFF000) >> 10));

	SetPageTableEntry((PageTableEntry *)pte, (u32)pAddr, PAGE_PRESENT | PAGE_WRITEABLE);
}

/*
	Met à jour le répertoire de pages passé en paramètre

	[!] L'adresse du répertoire de pages doit avoir été renseigné dans le registre cr3 au préalable !
*/
void AddPageToPageDirectory(u8 * vAddr, u8 * pAddr, PAGE_FLAG flags, PageDirectory pd)
{
	u32 * pde = NULL; // adresse virtuelle de l'entrée du répertoire de pages
	u32 * pte = NULL; // adresse virtuelle de l'entrée de la table de pages
	u32 * pt = NULL;  // adresse virtuelle de la table de pages

	// On vérifie que la page est bien présente (voir get_p_addr pour mieux comprendre l'algo)
	pde = (u32 *)(0xFFFFF000 | PD_OFFSET((u32)vAddr));

	if (!FlagOn(*pde, PAGE_PRESENT))
	{
		Page new_page = PageAlloc();
		pt = (u32 *)new_page.vAddr;

		// On ajoute la nouvelle page au répertoire de pages
		SetPageDirectoryEntry((PageDirectoryEntry *)pde, (u32)new_page.pAddr, PAGE_PRESENT | PAGE_WRITEABLE | flags);

		CleanPageTable((PageTableEntry *)pt);

		// On ajoute la nouvelle page à la liste de pages associée à ce répertoire (pour plus facilement libérer la mémoire après)
		ListPush(pd.pageTableList, new_page.vAddr);
	}

    pte = (u32 *)(0xFFC00000 | (((u32)vAddr & 0xFFFFF000) >> 10));

	SetPageTableEntry((PageTableEntry *)pte, (u32)pAddr, (PAGE_PRESENT | PAGE_WRITEABLE | flags));
}

PageDirectory CreateProcessPageDirectory()
{
	Page pd_page = PageAlloc();
	PageDirectory pd = { 0 };
	PageDirectoryEntry * kernelPdEntry = (PageDirectoryEntry *)gKernelInfo.pPageDirectory.pdEntry;
	PageDirectoryEntry * pdEntry = (PageDirectoryEntry *)pd_page.vAddr;
	unsigned int i = 0;
	
	// On veut que le premier Go de mémoire virtuelle soit pour le noyau : 1024 / 4 = 256 (1024 = nombre d'entrées dans un répertoire de pages)
	// On vérifie:  256 * 1024 * 4096 = 1Go
	for (; i < NB_PAGES_TABLE_PER_KERNEL_DIRECTORY; i++)
		pdEntry[i] = kernelPdEntry[i];

	for (i = 256; i < NB_PAGES_TABLE_PER_DIRECTORY; i++)
		SetPageDirectoryEntry(&(pdEntry[i]), 0, PAGE_WRITEABLE | PAGE_NON_PRIVILEGED_ACCESS);

	SetPageDirectoryEntry(&(pdEntry[1023]), (u32)pd_page.pAddr, PAGE_PRESENT | PAGE_WRITEABLE);

	pd.pdEntry = (PageDirectoryEntry *)pd_page.pAddr;
	pd.pageTableList = ListCreate();

	return pd;
}

BOOL IsVirtualAddressAvailable(u32 vAddr)
{
	u32 * pde = NULL; // adresse virtuelle de l'entrée du répertoire de pages
	u32 * pte = NULL; // adresse virtuelle de l'entrée de la table de pages

	// On vérifie que la page est bien présente (voir get_p_addr pour mieux comprendre l'algo)
	pde = (u32 *)(0xFFFFF000 | PD_OFFSET(vAddr));
	if (!FlagOn(*pde, PAGE_PRESENT))
	{
		return FALSE;
	}

	pte = (u32 *)(0xFFC00000 | ((vAddr & 0xFFFFF000) >> 10));
	if (!FlagOn(*pte, PAGE_PRESENT))
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CheckUserVirtualAddressValidity(u32 vAddr)
{
	return (vAddr != 0 && vAddr >= V_USER_BASE_ADDR);
}

void SaveCurrentMemoryMapping()
{
	__debugbreak();

	s_SavedPageDirectoryEntry = _getCurrentPagesDirectory();

	KLOG(LOG_DEBUG, "Save page dir %x", s_SavedPageDirectoryEntry);

	if (s_SavedPageDirectoryEntry != NULL)
	{
		KLOG(LOG_WARNING, "The s_SavedPageDirectoryEntry is already used");
		//__debugbreak();
	}
}

void RestoreMemoryMapping()
{
	if (s_SavedPageDirectoryEntry != NULL)
	{
		_setCurrentPagesDirectory(s_SavedPageDirectoryEntry);
		s_SavedPageDirectoryEntry = NULL;
	}
}