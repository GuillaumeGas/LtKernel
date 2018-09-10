#pragma once

#include <kernel/init/memory.h>

struct mem_block * ksbrk(int n);
void * kmalloc(int size);
void kfree(void * ptr);

void * page_alloc();
void page_free(void * ptr);