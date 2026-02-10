#ifndef VM_H
#define VM_H

#include "types.h"
#include "mmu.h"

pte_t *walkpgdir(pde_t *pgdir, const void *va, int alloc);

void init_swap_bitmap(void);
void init_vm_resources(void);

void swap_out(void);
int allocate_swap_block(void);

void swap_in(pde_t *pgdir, void *vaddr);

extern struct spinlock lru_lock;
extern struct spinlock swap_lock;


#endif
