#pragma once

#include <kernel/init/vmm.h>

#define KERNEL_PAGE_DIR_P_ADDR 0x1000            // Adresse du répertoire de pages du noyau
#define KERNEL_PAGES_TABLE_P_ADDR 0x400000       // Adresse des tables de pages du noyau
#define KERNEL_LIMIT_P_ADDR 0x800000             // Adresse limite du noyau
#define KERNEL_STACK_P_ADDR 0xA0000              // Adresse pile noyau
#define KERNEL_PAGES_HEAP_V_BASE_ADDR 0x800000   // Adresse virtuelle tas de pages
#define KERNEL_PAGES_HEAP_V_LIMIT_ADDR 0x1000000 // Adresse limite virtuelle tas de pages
#define KERNEL_HEAP_V_BASE_ADDR 0x10000000		 // Adresse virtuelle de base du tas
#define KERNEL_HEAP_V_LIMIT_ADDR 0x40000000      // Adresse virtuelle limite du tas

struct KernelInfo
{
	PageDirectory pageDirectory_p;
	PageTableEntry * pageTables_p;
	u32 kernelLimit_p;
	u32 stackAddr_p;
	u32 pagesHeapBase_v;
	u32 pagesHeapLimit_v;
	u32 heapBase_v;
	u32 heapLimit_v;
} typedef KernelInfo;

#ifdef __KERNEL__
KernelInfo g_kernelInfo = { 0 };
#else
extern KernelInfo g_kernelInfo;
#endif