#pragma once

#include <kernel/init/vmm.h>

#define KERNEL_PAGE_DIR_P_ADDR         0x1000     // Adresse du répertoire de pages du noyau
#define KERNEL_PAGES_TABLE_P_ADDR      0x400000   // Adresse des tables de pages du noyau
#define KERNEL_LIMIT_P_ADDR            0x800000   // Adresse limite du noyau
#define KERNEL_STACK_P_ADDR            0xA0000    // Adresse pile noyau
#define KERNEL_PAGES_HEAP_V_BASE_ADDR  0x800000   // Adresse virtuelle tas de pages
#define KERNEL_PAGES_HEAP_V_LIMIT_ADDR 0x1000000  // Adresse limite virtuelle tas de pages
#define KERNEL_HEAP_V_BASE_ADDR        0x10000000 // Adresse virtuelle de base du tas
#define KERNEL_HEAP_V_LIMIT_ADDR       0x40000000 // Adresse virtuelle limite du tas

struct KernelInfo
{
	PageDirectory pPageDirectory;
	PageTableEntry * pPageTables;
	u32 pKernelLimit;
	u32 pStackAddr;
	u32 vPagesHeapBase;
	u32 vPagesHeapLimit;
	u32 vHeapBase;
	u32 vHeapLimit;
} typedef KernelInfo;

typedef void(*CleanCallbackFun)(void);

#ifdef __KERNEL__
KernelInfo gKernelInfo = { 0 };
#else
extern KernelInfo gKernelInfo;
#endif