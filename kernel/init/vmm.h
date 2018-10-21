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

#define MEM_BITMAP_SIZE RAM_MAXPAGE / 8

#define PAGE(addr) (addr) >> 12
#define PD_OFFSET(addr) ((addr) & 0xFFC00000) >> 20
#define PT_OFFSET(addr) (addr & 0x003FF000) >> 12

enum PAGE_FLAG
{
	PAGE_EMPTY = 0,
	PAGE_PRESENT = 1,
	PAGE_WRITEABLE = 2,
	PAGE_NON_PRIVILEGED_ACCESS = 4,
	PAGE_PWT = 8,
	PAGE_PCD = 16,
	PAGE_ACCESSED = 32,
	PAGE_PAGE_SIZE_4MO = 128,
	PAGE_WRITTEN = 64,
	PAGE_G = 256
} typedef PAGE_FLAG;

struct page_directory_entry
{
	u32 present : 1;
	u32 writable : 1;
	u32 non_privileged_access : 1;
	u32 pwt : 1;
	u32 pcd : 1;
	u32 accessed : 1;
	u32 reserved : 1;
	u32 page_size : 1;
	u32 global : 1;
	u32 avail : 3;
	u32 page_table_addr : 20;
};
typedef struct page_directory_entry PageDirectoryEntry;

struct page_table_entry
{
	u32 present : 1;
	u32 writable : 1;
	u32 non_privileged_access : 1;
	u32 pwt : 1;
	u32 pcd : 1;
	u32 accessed : 1;
	u32 written : 1;
	u32 reserved : 1;
	u32 global : 1;
	u32 avail : 3;
	u32 page_addr : 20;
};
typedef struct page_table_entry PageTableEntry;

struct page_directory
{
	PageDirectoryEntry * pd_entry;
	List * page_table_list;
};
typedef struct page_directory PageDirectory;

struct page
{
	u32 * p_addr;
	u32 * v_addr;
};
typedef struct page Page;

void InitVmm();

void CleanAllPageDirectoryAndPageTables(PageDirectoryEntry * pageDirectoryEntry, PageTableEntry * pageTableEntry);
void CleanPageDirectory(PageDirectoryEntry * pageDirectory);
void CleanPageTable(PageTableEntry * pageTable);
void SetPageDirectoryEntry(PageDirectoryEntry * pde, u32 pt_addr, PAGE_FLAG flags);
void SetPageDirectoryEntryEx(PageDirectoryEntry * pde, u32 pt_addr, PAGE_FLAG flags, u8 global, u8 avail);
void SetPageTableEntry(PageTableEntry * pt, u32 page_addr, PAGE_FLAG flags);
void SetPageTableEntryEx(PageTableEntry * pt, u32 page_addr, PAGE_FLAG flags, u8 global, u8 avail);
void * GetFreePage();
void ReleasePage(void * p_addr);
void * GetPhysicalAddress(void * v_addr);

void AddPageToKernelPageDirectory(u8 * v_addr, u8 * p_addr, PAGE_FLAG flags);
void AddPageToPageDirectory(u8 * v_addr, u8 * p_addr, PAGE_FLAG flags, PageDirectory pd);

PageDirectory CreateProcessPageDirectory();

void VmmCleanCallback();