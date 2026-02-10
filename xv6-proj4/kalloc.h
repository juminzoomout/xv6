#ifndef KALLOC_H
#define KALLOC_H

#include "mmu.h"
#include "types.h"

extern struct page *page_lru_head;

void add_to_lru(struct page *page);
void remove_from_lru(struct page *page);

#endif
