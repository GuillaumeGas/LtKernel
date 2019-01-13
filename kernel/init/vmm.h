#pragma once

#include <kernel/lib/types.h>
#include <kernel/lib/list.h>
#include <kernel/lib/stdlib.h>
#include <kernel/init/heap.h>

#define PAGE_SIZE 4096        // Taille d'une page
#define RAM_MAXPAGE 0x100000  // Car on permet d'adresser 4Go de mémoire (0x100000 * 0x1000 = 4Go)

#define PAGING_FLAG 0x80000000 // CR0 - bit 31

#define NB_PAGES_TABLE_PER_DIRECTORY 1024
#define EMPTY_PAGE_TABLE 0
#define NB_PAGES_PER_TABLE 1024
#define NB_PAGES_TABLE_PER_KERNEL_DIRECTORY 256 // On veut que le premier Go de mémoire virtuelle soit pour le noyau : 1024 / 4 = 256 (1024 = nombre d'entrées dans un répertoire de pages)
                                                // On vérifie:  256 * 1024 * 4096 = 1Go

#define MEM_BITMAP_SIZE RAM_MAXPAGE / 8

#define PAGE(addr) (addr) >> 12
#define PD_OFFSET(addr) ((addr) & 0xFFC00000) >> 20
#define PT_OFFSET(addr) (addr & 0x003FF000) >> 12

#define V_USER_BASE_ADDR 0x40000000

enum PAGE_FLAG
{
	PAGE_EMPTY = 0,
	PAGE_PRESENT = 1,
	PAGE_WRITEABLE = 2,
	PAGE_NON_PRIVILEGED_ACCESS = 4,
	PAGE_PWT = 8,
	PAGE_PCD = 16,
	PAGE_ACCESSED = 32,
	PAGE_SIZE_4MO = 128,
	PAGE_WRITTEN = 64,
	PAGE_G = 256
} typedef PAGE_FLAG;

struct page_directory_entry
{
	u32 present : 1;
	u32 writable : 1;
	u32 nonPrivilegedAccess : 1;
	u32 pwt : 1;
	u32 pcd : 1;
	u32 accessed : 1;
	u32 reserved : 1;
	u32 pageSize : 1; // 0 : 4Ko, 1 : 4Mo
	u32 global : 1;
	u32 avail : 3;
	u32 pageTableAddr : 20;
};
typedef struct page_directory_entry PageDirectoryEntry;

struct page_table_entry
{
	u32 present : 1;
	u32 writable : 1;
	u32 nonPrivilegedAccess : 1;
	u32 pwt : 1;
	u32 pcd : 1;
	u32 accessed : 1;
	u32 written : 1;
	u32 reserved : 1;
	u32 global : 1;
	u32 avail : 3;
	u32 pageAddr : 20;
};
typedef struct page_table_entry PageTableEntry;

struct page_directory
{
	PageDirectoryEntry * pdEntry;
	List * pageTableList;
};
typedef struct page_directory PageDirectory;

struct page
{
	u32 * pAddr;
	u32 * vAddr;
};
typedef struct page Page;

void InitVmm();

void CleanAllPageDirectoryAndPageTables(PageDirectoryEntry * pageDirectoryEntry, PageTableEntry * pageTableEntry);
void CleanPageDirectory(PageDirectoryEntry * pageDirectory);
void CleanPageTable(PageTableEntry * pageTable);
void SetPageDirectoryEntry(PageDirectoryEntry * pde, u32 ptAddr, PAGE_FLAG flags);
void SetPageDirectoryEntryEx(PageDirectoryEntry * pde, u32 ptAddr, PAGE_FLAG flags, u8 global, u8 avail);
void SetPageTableEntry(PageTableEntry * pt, u32 pageAddr, PAGE_FLAG flags);
void SetPageTableEntryEx(PageTableEntry * pt, u32 pageAddr, PAGE_FLAG flags, u8 global, u8 avail);
void * GetFreePage();
void ReleasePage(void * pAddr);
void * GetPhysicalAddress(void * vAddr);
BOOL IsVirtualAddressAvailable(u32 vAddr);
BOOL CheckUserVirtualAddressValidity(u32 vAddr);

void AddPageToKernelPageDirectory(u8 * vAddr, u8 * pAddr, PAGE_FLAG flags);
void AddPageToPageDirectory(u8 * vAddr, u8 * pAddr, PAGE_FLAG flags, PageDirectory pd);

PageDirectory CreateProcessPageDirectory();

void VmmCleanCallback();

void _setCurrentPagesDirectory(PageDirectoryEntry * pd);
PageDirectoryEntry * _getCurrentPagesDirectory();