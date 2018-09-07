#pragma once

#include <kernel/lib/types.h>

#define KERNEL_STACK_P_ADDR 0xA0000

#define PAGING_FLAG 0x80000000 // CR0 - bit 31

#define PD0_ADDR 0x1000 // adresse 1er répertoire de pages

#define NB_PAGES_TABLE_PER_DIRECTORY 1024
#define EMPTY_PAGE_TABLE 0
#define NB_PAGES_PER_TABLE 1024
#define PAGE_SIZE 4096
//#define RAM_MAXPAGE NB_PAGES_TABLE_PER_DIRECTORY * NB_PAGES_PER_TABLE
#define RAM_MAXPAGE 0x10000
#define MEM_BITMAP_SIZE RAM_MAXPAGE / 8

#define HEAP_BASE_ADDR  0x10000000
#define HEAP_LIMIT_ADDR 0x40000000

#define PAGE_HEAP_BASE_ADDR  0x800000
#define PAGE_HEAP_LIMIT_ADDR 0x1000000

#define BLOCK_HEADER_SIZE sizeof(int)
#define DEFAULT_BLOCK_SIZE PAGE_SIZE
#define DEFAULT_BLOCK_SIZE_WITH_HEADER DEFAULT_BLOCK_SIZE + BLOCK_HEADER_SIZE

#define MINIMAL_BLOCK_SIZE 4
#define BLOCK_FREE 0
#define BLOCK_USED 1


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

struct mem_block
{
	unsigned int size : 31; // 31 bits pour la taille, 1 bit pour indiquer si le bloc est libre (0) ou non (1)
	unsigned int state : 1;
	void * data;
};

void init_vmm();
void init_heap();

void init_pages_directory(struct page_directory_entry * first_pd);
void set_page_directory_entry(struct page_directory_entry * pd, u32 pt_addr, PD_FLAG flags);
void set_page_directory_entryEx(struct page_directory_entry * pd, u32 pt_addr, PD_FLAG flags, u8 global, u8 avail);
void set_page_table_entry(struct page_table_entry * pt, u32 page_addr, PT_FLAG flags);
void set_page_table_entryEx(struct page_table_entry * pt, u32 page_addr, PT_FLAG flags, u8 global, u8 avail);
void * get_free_page();

#ifdef __MEMORY__
struct page_directory_entry * kernel_pd = NULL;
struct page_table_entry * kernel_pt = NULL;

struct mem_block * g_heap = NULL;
struct mem_block * g_last_heap_block = NULL;
#else
extern struct page_directory_entry * kernel_pd;
extern struct page_table_entry * kernel_pt;

extern struct mem_block * g_heap;
extern struct mem_block * g_last_heap_block;
#endif