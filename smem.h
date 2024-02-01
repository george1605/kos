#pragma once
#define M_VALID 1
#define M_READ 2
#define M_WRITE 4
#define M_BOTH 16
#define M_USED 32
#define M_STATIC 128
#define PAGE_PRESENT  0x1
#define PAGE_WRITABLE 0x2
#define PAGE_USER     0x4

#define PAGE_KERNEL       0x01
#define PAGE_WRITABLE     0x02
#define PAGE_NOCACHE      0x04
#define PAGE_WRITETHROUGH 0x08
#define PAGE_SPEC         0x10
#define PAGE_WC           (MMU_FLAG_NOCACHE | MMU_FLAG_WRITETHROUGH | MMU_FLAG_SPEC)
#define PAGE_NOEXECUTE    0x20
#define PAGE_GET_MAKE     0x01
#include "lib.c"
#include "mem.h"
#define PAGE_ALLIGN(x) (void *)(x - x % 4096 + 4096)
#define SETUP_HEAP(size) __attribute__((section(".heap"))) char memory[size]
#define RAND_INIT(mem, size) memset(mem, 0xCD, size)
size_t alig = 0;
int *memcbrk = (int *)0x5E0000;
int *heapend = (int *)0x3E00000;

struct kmap
{
  long virt;
  void *physs;
  void *physe;
  int perm;
};

struct MemChunk
{
  int *start;
  int size;
  int flags;
  int seek;
};

struct binheap 
{
  char* mem;
  int size;
  int flags;
};

struct malloc_block {
    uint32_t size;  // Size of this memory block in bytes
    char free;      // Is this block of memory free?
    void* mem;
    struct malloc_block *next;  // Next block of memory
};

static struct malloc_block *malloc_head = 0;    // Start of linked list
static struct malloc_block *malloc_cnt = malloc_head;    // Start of linked list

struct binheap binalloc(int size)
{
  struct binheap u;
  u.mem = alloc(0, size + 1);
  u.size = size;
  u.flags = M_BOTH | M_STATIC;
  *u.mem = 0xA0;
  return u;
}

void binfree(struct binheap u)
{
  if(u.mem == NULL_PTR) return;
  memset(u.mem, 0x00, u.size);
  u.mem = NULL_PTR;
  u.size = 0;
}

void extheap(size_t bytes)
{
  *heapend = -1;
  heapend += bytes;
  if (heapend >= 0x6000000) // can be extended only 44 MB
    heapend = 0x6000000;
  *heapend = 0;
}

struct MemChunk *memalloc(size_t bytes)
{
  struct MemChunk *u = (struct MemChunk *)memcbrk;
  u->size = bytes;
  u->flags = M_BOTH;
  u->seek = 0;
  u->start = alloc(0, bytes);
  memcbrk += (1 + sizeof(struct MemChunk));
  return u;
}

void* safe_alloc_k(void* start, size_t size)
{
   if(malloc_head == 0)
   {
      malloc_head = memcbrk;
      malloc_cnt = malloc_head;
   }
   malloc_cnt->next = malloc_cnt + sizeof(struct malloc_block) + 1;
   malloc_cnt = malloc_cnt->next;
   malloc_cnt->size = size;
   malloc_cnt->free = 0;
   malloc_cnt->mem  = alloc(start, size);
   return malloc_cnt->mem;
}

void* safe_alloc(size_t size)
{
  void* start = vmap(NULL_PTR, size, PAGE_PRESENT, (struct vfile*)NULL_PTR);
  return safe_alloc_k(start, size);  
}

void *dyalloc(size_t off)
{ // page aligned pointer
  int u = 0x300000 + off;
  if (u % 4096 > 0)
  {
    u -= (u % 4096);
    u += 4096;
  }
  *(int *)(u - 1) = 0xdeadbeef;
  return (void *)u;
}

void *algalloc()
{
  void *u = dyalloc(alig);
  alig += 4096;
  *(char *)(u + 4095) = 0;
  return u;
}

int memgrow(struct MemChunk *u, size_t bytes)
{
  if (u->flags & M_STATIC || bytes < u->size)
    return -1;

  int i, *p;
  for (i = 0; i < (bytes - u->size); i++)
  {
    if (*(u->start + u->size + i) == 0xdeadbeef)
    {
      p = kalloc(bytes, USER_MEM);
      memcpy(p, u->start, u->size);
      memset(u->start, 0, u->size);
      u->size = bytes;
      u->start = p;
      return 1;
    }
  }
  *(u->start + bytes) = 0;
  return 0;
}

void memgap(size_t i)
{
  heapbrk += i;
}

void memfree(struct MemChunk *u)
{
  if (u != 0 && u->start > 1)
  {
    free(u->start);
    free(u);
  }
}

void memseek(struct MemChunk *u, int seek)
{
  if (seek < (u->size))
    u->seek = seek;
}

int memread(struct MemChunk *u)
{
  if (u != 0 && (u->flags & M_READ))
    return *(u->start + u->seek);
  else
    return -1;
}

void memwrite(struct MemChunk *u, int value)
{
  if (u != 0 && (u->flags & M_WRITE))
    *(u->start + u->seek) = value;
}

#define _malloc(x) memalloc(x)
#define _free(x) memfree(x)

struct heapblk
{
  size_t size;
  int flags;
  void *ptr;
  struct heapblk *next;
  struct heapblk *prev;
  struct heapblk *head;
} cblk;

struct farptr
{
  uint16_t base;
  uint16_t off;
};

typedef uint32_t pagetblentry_t;

typedef struct {
    pagetblentry_t entries[1024];
} pagetable;

void *getptr(struct farptr u)
{
  void *ptr = (void *)(u.base * 0x10 + u.off);
  return ptr;
}

struct heapblk* heapnew(int bytes)
{
  struct heapblk *n = (struct heapblk *)0x7F0000;
  n->ptr = alloc(0, bytes);
  n->size = bytes;
  n->flags = M_USED | M_BOTH;
  cblk.next = n;
  cblk = *n;
  return n;
}

struct heapblk* heapmerge(struct heapblk *a, struct heapblk *b)
{
  struct heapblk* n = heapnew(a->size + b->size);
  n->ptr = a->ptr;
  a->flags |= M_VALID;
  b->flags |= M_VALID;
  free(a), free(b);
}

int heaptrv(int dir, struct heapblk *u)
{ // gets the number of heap blocks
  if (u == 0)
  {
    return 0;
  }
  else if (dir == 1)
  {
    return 1 + heaptrv(dir, u->next);
  }
  else
  {
    return 1 + heaptrv(dir, u->prev);
  }
}

void heapadd(struct heapblk *u, struct heapblk *i)
{
  i->next = u;
  u->prev = i;
}

void hmerge(struct heapblk* blk)
{
  if(blk->flags != blk->next->flags)
    return;
  
  void* p = blk->next;
  blk->size += blk->next->size;
  blk->next = blk->next->next;
  free(p); // what ?
}

struct heapblk *halloc(int bytes)
{
  struct heapblk *start = cblk.next;
  while (start != 0) // checks if a heap block was freed
  {
    if ((start->flags & M_VALID) && (bytes <= start->size))
    {
      start->flags &= M_VALID;
      start->flags |= M_USED;
      start->size = bytes;
      memset(start->ptr, 0, bytes);
      return start;
    }
    start = start->next; // goes to the next element in list
  }
  struct heapblk *n = (struct heapblk *)0x7F0000;
  n->ptr = alloc(0, bytes);
  n->size = bytes;
  n->flags = M_USED | M_BOTH;
  start->next = n; // appends a new heapblkl structure
  return n;
}

void hfree(struct heapblk *u)
{
  int x = strlen((char *)u->ptr);
  memset(u->ptr, 0, x);
  u->flags &= M_USED;
  u->flags |= M_VALID;
}

int *newint(int value)
{ 
  int *k = (int *)kalloc(sizeof(int), USER_MEM);
  *k = value;
  return k;
}

char *newchar(char value)
{
  char *k = (char *)kalloc(1, USER_MEM);
  *k = value;
  return k;
}

char *strdup(char *x)
{ // allocates a new string and copies from the original string
  if (x == 0)
    return (char *)0;

  int len = strlen(x);
  char *ptr = (char*)kalloc(len + 1, USER_MEM);
  memcpy(ptr, x, len);
  ptr[len - 1] = '\0';
  return ptr;
}

void freerange(void *from, void *to)
{
  if(from < _vm(0) || to < _vm(0)) return;
  long x = (long)from;
  while(x < to)
  {
    free(PAGE_ALLIGN(x));
    x += 4096;
  }
}

void* map_page(void* physaddr, void* virtualaddr, size_t flags) 
{
    uint64_t pdindex = (uint64_t)virtualaddr >> 22;
    uint64_t ptindex = (uint64_t)virtualaddr >> 12 & 0x03FF;

    uint64_t* pd = (uint64_t*)0xFFFFF000;
    uint64_t* pt = ((uint64_t*)0xFFC00000) + (0x400 * pdindex);
    pt[ptindex] = ((uint64_t)physaddr) | (flags & 0xFFF) | 0x01; 
    return virtualaddr;
}

void* mmu_add_ptable(pagetable* tbl, void* virt, void* phys, size_t flags)
{
  uint64_t ptindex = ((uint64_t)virt >> 12) & 0x03FF;
  tbl->entries[ptindex] = ((uint64_t)phys) | (flags & 0xFFF) | 0x01;
  return virt;
}

void *get_phys(void *virtualaddr)
{
  uint64_t pdindex = (uint64_t)virtualaddr >> 22;
  unsigned long ptindex = (unsigned long)virtualaddr >> 12 & 0x03FF;

  unsigned long *pd = (unsigned long *)0xFFFFF000;
  unsigned long *pt = ((unsigned long *)0xFFC00000) + (0x400 * pdindex);

  return (void *)((pt[ptindex] & ~0xFFF) + ((unsigned long)virtualaddr & 0xFFF));
}

void enable_paging(uint64_t address)
{
  int __ignore;
  asm volatile("movl %0, %%cr3":"r"(address), "=r"(__ignore));
}

void mmu_init()
{
  asm volatile (
		"movq %%cr0, %%rax\n"
		"orq  $0x10000, %%rax\n"
		"movq %%rax, %%cr0\n"
		: : : "rax");
}

static inline void tlb_flush(uint64_t addr) // invalidates a page
{
  asm volatile("invlpg (%0)" ::"r"(addr)
               : "memory");
}

struct {
  struct spinlock kern_lock;
  struct spinlock mmio_lock;
  struct spinlock mod_lock; // module space, ELFs
} memlocks;

uint64_t mmu_allocate_a_page()
{
  // TO DO
  pagetable* tbl;
  return 0;
}

char * mmio_base = (char*)DRIV_MEM;
void * mmu_map_mmio_region(uint64_t physical_address, size_t size) {
	spin_lock(&memlocks.mmio_lock);
	void * out = (void*)mmio_base;
	for (size_t i = 0; i < size; i += PAGE_SIZE) {
		struct pagtable* p = mmu_get_page(mmio_base + i, PAGE_GET_MAKE);
		mmu_frame_map_address(p, PAGE_KERNEL | PAGE_WRITABLE | PAGE_NOCACHE | PAGE_WRITETHROUGH, physical_address + i);
	}
	mmio_base += size;
	spin_unlock(&memlocks.mmio_lock);

	return out;
}

static uint64_t allocate_page(uint64_t * phys_out) {
	uint64_t phys = mmu_allocate_a_frame() << 12;
	uint64_t virt = (uint64_t)mmu_map_mmio_region(phys, 4096);
	memset((void*)virt,0,4096);
	*phys_out = phys;
	return virt;
}

void mem_unmap(pagetable* table)
{
  CLI();
  for(int i = 0;i < 1024;i++)
  {
    if (table->entries[i] & PAGE_PRESENT) {
        uint32_t page_table_address = table->entries[i] & 0xFFFFF000;
        pagetable* page_table = (pagetable*)page_table_address;
        for (int j = 0; j < 1024; ++j) {
            page_table->entries[j] = 0;
        }
        table->entries[i] = 0;
    }
  }
  tlb_flush((size_t)table);
  STI();
}

/*
 * Linux kernel function, using get_phys to get physical page
 * and memcpy to copy to kernel address
*/
#define MEM_SET_DEFAULT 0x0
#define MEM_MAKE_ALIGNED 0x1
#define MEM_CHECK_VPTR 0x2

void copy_from_user(char* kptr, const void* uptr, size_t size)
{
  void* phys = get_phys(uptr);
  if(phys == NULL_PTR)
    return;

  memcpy(kptr, phys, size);
}

void copy_from_user_align(char* kptr, const void* uptr, size_t size, uint16_t align)
{
  if(size == 0) return;
  size = size - size % align + align;
  copy_from_user(kptr, uptr, size);
}

/*
 * This can be used for kernel modules
*/
#define KERN_START_ADDR 0x100000

void copy_into_kernel(uint32_t offset, const void* ptr, size_t size)
{
  void* kptr = (void*)(KERN_MEM + offset);
  copy_from_user((char*)kptr, ptr, size);
}

uint64_t mmu_first_n_frames(int n) {
	for (uint64_t i = 0; i < nframes * PAGE_SIZE; i += PAGE_SIZE) {
		int bad = 0;
		for (int j = 0; j < n; ++j) {
			if (mmu_frame_test(i + PAGE_SIZE * j)) {
				bad = j + 1;
			}
		}
		if (!bad) {
			return i / PAGE_SIZE;
		}
	}
	return (uint64_t)-1;
}


typedef struct {
  void* ptr;
  size_t size;
  int free;
  char multi; // if multiblocks are used: 0 0 (1 1 1 1) <-- 4 blocks
} arenablk;

typedef struct {
  arenablk* blocks;
  int num_blocks;
  pagetable* tbl;
} arena;

typedef struct {
  arenablk* blocks;
  int num_blocks;
  pagetable* tbl;
  struct spinlock lock;
} karena;

void* arena_alloc(arena* ar, size_t size, void*(*def_alloc)(size_t))
{
  for(int i = 0;i < ar->num_blocks;i++) {
    if(ar->blocks[i].free == 0 && ar->blocks[i].size == size) {
      ar->blocks[i].multi = 1;
      return ar->blocks[i].ptr;
    }
  }
  
  return def_alloc(size);
}

void* karena_alloc(karena* ar, size_t size)
{
  void* p;
  acquire(&(ar->lock));
  p = arena_alloc((arena*)ar, size, safe_alloc);
  release(&(ar->lock));
  return p;
}

// finds the first block and tries to merge it with next blocks
// if it can't then it uses def_alloc(size);
void* arena_realloc(arena* ar, void* p, size_t size, void*(*def_realloc)(void*, size_t))
{
  int start = 0, sz = 0;
  for(int i = 0;i < ar->num_blocks;i++)
    if(abs((size_t)ar->blocks[i].ptr - (size_t)p) < ar->blocks[i].size)
        start = i, sz = ar->blocks[i].size;
  while(sz < size)
  {
    sz += ar->blocks[start].size;
    ar->blocks[start++].free = 1;
    ar->blocks[start].multi = 1;
  }
  if(abs(sz - size) > size)
    return p;
  return def_realloc(p, size);
}

void arena_free(arena* ar, void* p)
{
  int start = 0;
  for(int i = 0;i < ar->num_blocks;i++)
    if(abs((size_t)ar->blocks[i].ptr - (size_t)p) < ar->blocks[i].size)
        start = i;
  while(ar->blocks[start].multi == 1)
  {
    ar->blocks[start++].free = 0;
  }
}

// blocks of size 1,2,4,8..2^n
arena* arena_setup(pagetable* tbl, size_t numblks)
{
  arena* ar = (arena*)kalloc(sizeof(arena), KERN_MEM);
  ar->blocks = (arenablk*)kalloc(sizeof(arenablk) * numblks, KERN_MEM);
  int p = 1;
  for(int i = 0;i < numblks;i++)
  {
    ar->blocks[i].free = 1;
    ar->blocks[i].size = p;
    ar->blocks[i].multi = 0;
    ar->blocks[i].ptr = vmap(NULL_PTR, p, PAGE_USER | PAGE_WRITABLE, (struct vfile*)NULL_PTR);
    p *= 2;
  }
  ar->num_blocks = numblks;
  return ar;
}

// Allocates a chunk from the arena or gets a kernel
arena* arena_setup2(size_t numblks)
{
  arena* ar = (arena*)kalloc(sizeof(arena), KERN_MEM);
  ar->blocks = (arenablk*)kalloc(sizeof(*ar->blocks) * numblks, KERN_MEM);
  uint64_t size = fastpow(2, numblks);
  void* bigblk = vmap(NULL_PTR, size, PAGE_PRESENT | PAGE_WRITABLE, (struct vfile*)NULL_PTR);
  void* p = bigblk;
  for(int i = 0;i < numblks;i++) 
  {
    ar->blocks[i].free = 0;
    ar->blocks[i].multi = 0;
    ar->blocks[i].ptr = p;
    ar->blocks[i].size = size / 2;
    p += size / 2;
    size /= 2;
  }
  return ar;
}

// unmap page table, free the blocks and arena as a whole
void arena_delete(arena* ar)
{
  mem_unmap(ar->tbl);
  free(ar->blocks);
  free(ar);
}

void *ioremap(void* x, size_t len)
{
  void* y = 0xFFFFF + (x + y) >> 1;
  map_page(x, y, 0);
  *(char*)y = 0xA0;
  return y;
}

struct list_head {
	struct list_head *next, *prev;
};

struct list_elem {
  struct list_elem *next, *prev;
  void* data;
};

void list_init(struct list_head* head)
{
  head->next = NULL_PTR;
  head->prev = NULL_PTR;
}

void list_add(struct list_head* head, struct list_head* elem)
{
  head->next = elem;
  elem->prev = head;
}

struct list_head* list_alloc_k(size_t elems)
{
  int i;
  struct list_head* head = kalloc(sizeof(struct list_head), KERN_MEM), *p;
  p = head;
  head->prev = NULL_PTR;
  for(i = 0;i < elems;i++) {
    list_add(p, kalloc(sizeof(struct list_elem), KERN_MEM));
    p = p->next;
  }
  p->next = NULL_PTR;
  return head;
}

void list_free_k(struct list_head* head)
{
  assert(head != NULL_PTR);
  struct list_head* h;
  while(head->next != NULL_PTR)
  {
    h = head->next;
    free(head->next);
    head->next = h;
  }
}