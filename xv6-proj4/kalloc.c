// Physical memory allocator, intended to allocate
// memory for user processes, kernel stacks, page table pages,
// and pipe buffers. Allocates 4096-byte pages.

#include "types.h"
#include "defs.h"
#include "vm.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"
#include "proc.h"
#include "x86.h"

void freerange(void *vstart, void *vend);
extern char end[]; // first address after kernel loaded from ELF file
                   // defined by the kernel linker script in kernel.ld

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  int use_lock;
  struct run *freelist;
} kmem;

struct page pages[PHYSTOP/PGSIZE];
struct page *page_lru_head;
int num_free_pages;
int num_lru_pages;

void add_to_lru(struct page *page) {
  acquire(&lru_lock);
  if(!page_lru_head) {
    page_lru_head = page;
    page->next = page;
    page->prev = page;
  }
  else {
    struct page *tail = page_lru_head->prev;
    tail->next = page;
    page->prev = tail;
    page->next = page_lru_head;
    page_lru_head->prev = page;
  }
  release(&lru_lock);
}

void remove_from_lru(struct page *page) {
  acquire(&lru_lock);
  if(!page_lru_head || !page) {
    return;
  }

  if(page->next == page) {
    page_lru_head = 0;
  }
  else {
    page->prev->next = page->next;
    page->next->prev = page->prev;
    if(page == page_lru_head) {
      page_lru_head = page->next;
    }
  }
  page->next = 0;
  page->prev = 0;
  release(&lru_lock);
}


// Initialization happens in two phases.
// 1. main() calls kinit1() while still using entrypgdir to place just
// the pages mapped by entrypgdir on free list.
// 2. main() calls kinit2() with the rest of the physical pages
// after installing a full page table that maps them on all cores.
void
kinit1(void *vstart, void *vend)
{
  initlock(&kmem.lock, "kmem");
  kmem.use_lock = 0;
  freerange(vstart, vend);
}

void
kinit2(void *vstart, void *vend)
{
  freerange(vstart, vend);
  kmem.use_lock = 1;
}

void
freerange(void *vstart, void *vend)
{
  char *p;
  p = (char*)PGROUNDUP((uint)vstart);
  for(; p + PGSIZE <= (char*)vend; p += PGSIZE)
    kfree(p);
}
//PAGEBREAK: 21
// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(char *v)
{
  struct run *r;

  if((uint)v % PGSIZE || v < end || V2P(v) >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(v, 1, PGSIZE);

  if(myproc() != 0) {
    pte_t *pte = walkpgdir(myproc()->pgdir, v, 0);
    if(pte && (*pte & PTE_P)) {
      *pte = 0;
      if (*pte & PTE_U) {
        lcr3(V2P(myproc()->pgdir));
      }
    }
  }

  // rlu list에서 제거
  struct page *page = (struct page*)v;
  remove_from_lru(page);

  if(kmem.use_lock)
    acquire(&kmem.lock);

  r = (struct run*)v;
  r->next = kmem.freelist;
  kmem.freelist = r;
  if(kmem.use_lock)
    release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
char*
kalloc(void)
{
  struct run *r;

//try_again:
  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = kmem.freelist;
//  if(!r && reclaim())
//	  goto try_again;

  if(!r) {
    if(kmem.use_lock) {
      release(&kmem.lock);
    }
    swap_out();
    
    if(kmem.use_lock) {
      acquire(&kmem.lock);
    }
    if(!r) {
      if(kmem.use_lock) {
        release(&kmem.lock);
      }
      return 0;
    }
  }

  kmem.freelist = r->next;

  if(kmem.use_lock) {
    release(&kmem.lock);
  }

  // lru list에 추가
  struct page *page = (struct page*)r;
  
  if((uint)r < KERNBASE) {
    add_to_lru(page);
  }

  return (char*)r;


  if(kmem.use_lock)
    release(&kmem.lock);
  return (char*)r;
}

