#pragma once

#include <kernel/lib/types.h>
#include <kernel/lib/list.h>
#include <kernel/lib/stdlib.h>
#include <kernel/init/heap.h>

#define PAGE_SIZE 4096        // Taille d'une page
#define RAM_MAXPAGE 0x100000  // Car on permet d'adresser 4Go de mémoire (0x100000 * 0x1000 = 4Go)


//#define USER_TASK_P_ADDR 0x100000 // ne devrait plus être utilisée
#define USER_TASK_V_ADDR 0x40000000

#define USER_STACK_V_ADDR 0xE0000000

// Sélecteurs de segment de code et de pile d'une tâche utilisateur
// Les bits RPL (niveau de privilège) sont à '11' afin de permettre le passage en niveau de privilèges utilisateur
// lors de l'exécution de l'instruction iret (voir process_starter.asm et le fonctionnement de iret)
#define USER_CODE_SEG_SELECTOR 0x1B
#define USER_STACK_SEG_SELECTOR 0x23

#define PAGING_FLAG 0x80000000 // CR0 - bit 31

#define NB_PAGES_TABLE_PER_DIRECTORY 1024
#define EMPTY_PAGE_TABLE 0
#define NB_PAGES_PER_TABLE 1024

#define MEM_BITMAP_SIZE RAM_MAXPAGE / 8

// TODO : revoir organisation de la mémoire : ici on a un trou entre la table de pages du noyau et le tas de pages

#define BLOCK_HEADER_SIZE sizeof(int)
#define DEFAULT_BLOCK_SIZE PAGE_SIZE - BLOCK_HEADER_SIZE
#define DEFAULT_BLOCK_SIZE_WITH_HEADER DEFAULT_BLOCK_SIZE + BLOCK_HEADER_SIZE

#define MINIMAL_BLOCK_SIZE 4
#define BLOCK_FREE 0
#define BLOCK_USED 1

#define PAGE(addr) (addr) >> 12
#define PD_OFFSET(addr) (addr & 0xFFC00000) >> 20 // c'est pas plutôt >> 22 ?
#define PT_OFFSET(addr) (addr & 0x003FF000) >> 10 // >> 12 ?

// TODO : séparer les flags, on sait plus quel flag appartient à quel type...
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
typedef struct page_directory_entry PageDirectoryEntry;

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

void init_vmm();

void init_clean_pages_directory(PageDirectoryEntry * first_pd);
void init_clean_pages_table(PageTableEntry * first_pt);
void set_page_directory_entry(PageDirectoryEntry * pde, u32 pt_addr, PD_FLAG flags);
void set_page_directory_entryEx(PageDirectoryEntry * pde, u32 pt_addr, PD_FLAG flags, u8 global, u8 avail);
void set_page_table_entry(PageTableEntry * pt, u32 page_addr, PT_FLAG flags);
void set_page_table_entryEx(PageTableEntry * pt, u32 page_addr, PT_FLAG flags, u8 global, u8 avail);
void * get_free_page();
void release_page(void * p_addr);
void * get_p_addr(void * v_addr);

void pd0_add_page(u8 * v_addr, u8 * p_addr, PT_FLAG flags);
void pd_add_page(u8 * v_addr, u8 * p_addr, PT_FLAG flags, PageDirectory pd);

PageDirectory create_process_pd();

void VmmCleanCallback();