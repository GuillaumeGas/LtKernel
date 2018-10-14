#pragma once

#include <kernel/init/vmm.h>

struct mem_block * ksbrk(int n);
void * kmalloc(int size);
void kfree(void * ptr);

Page page_alloc();
void page_free(void * ptr);