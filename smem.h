#pragma once
#define M_VALID 1
#define M_READ 2
#define M_WRITE 4
#define M_BOTH 16
#define M_USED 32
#define M_STATIC 128
#include "lib.c"
#include "mem.h"
#define PAGE_ALLIGN(x) (void *)(x - x % 4096 + 4096)
#define SETUP_HEAP(size) __attribute__((section(".heap"))) char memory[size]
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

void *getptr(struct farptr u)
{
  void *ptr = (void *)(u.base * 0x10 + u.off);
  return ptr;
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
  char *ptr = kalloc(len, USER_MEM);
  memcpy(ptr, x, len);
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