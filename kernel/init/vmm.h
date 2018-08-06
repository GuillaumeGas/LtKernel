#pragma once

#include <kernel/lib/types.h>

#define PAGING_FLAG 0x80000000 // CR0 - bit 31

#define PD0_ADDR 0x20000 // adresse 1er répertoire de pages
#define PT0_ADDR 0x21000 // adresse 1ere table de pages

#define NB_PAGES_TABLE_PER_DIRECTORY 1024
#define EMPTY_PAGE_TABLE 0
#define NB_PAGES_PER_TABLE 1024
#define PAGE_SIZE 4096
//#define RAM_MAXPAGE NB_PAGES_TABLE_PER_DIRECTORY * NB_PAGES_PER_TABLE
#define RAM_MAXPAGE 0x10000
#define MEM_BITMAP_SIZE RAM_MAXPAGE / 8

enum PD_FLAG
{
	EMPTY = 0,
	IN_MEMORY = 2,
	WRITEABLE = 4,
	NON_PRIVILEGED_ACCESS = 8,
	PWT = 16,
	PCD = 32,
	ACCESSED = 64,
	WRITTEN = 128,
	PAGE_SIZE_4KO = 256,
	PAGE_SIZE_4MO = 512,
	G = 1024
};
typedef enum PD_FLAG PD_FLAG;
typedef enum PD_FLAG PT_FLAG;

struct page_directory_entry
{
	u32 in_memory : 1;
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

struct page_table_entry
{
	u32 in_memory : 1;
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

void init_vmm();

void init_pages_directory(struct page_directory_entry * first_pd);
void set_page_directory_entry(struct page_directory_entry * pd, u32 pt_addr, PD_FLAG flags);
void set_page_directory_entryEx(struct page_directory_entry * pd, u32 pt_addr, PD_FLAG flags, u8 global, u8 avail);
void set_page_table_entry(struct page_table_entry * pt, u32 page_addr, PT_FLAG flags);
void set_page_table_entryEx(struct page_table_entry * pt, u32 page_addr, PT_FLAG flags, u8 global, u8 avail);
void * get_free_page();

#ifdef __VMM__
struct page_directory_entry * kernel_pd = NULL;
struct page_table_entry * kernel_pt = NULL;
#else
extern struct page_directory_entry * kernel_pd;
extern struct page_table_entry * kernel_pt;
#endif