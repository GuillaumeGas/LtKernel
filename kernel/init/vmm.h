#pragma once

#include <kernel/lib/types.h>

#define PAGING_FLAG 0x80000000 // CR0 - bit 31

#define PD0_ADDR 0x20000 // adresse 1er répertoire de pages
#define PT0_ADDR 0x21000 // adresse 1ere table de pages

#define NB_PAGES_TABLE_PER_DIRECTORY 1024
#define EMPTY_PAGE_TABLE 0
#define NB_PAGES_PER_TABLE 1024
#define PAGE_SIZE 4096

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