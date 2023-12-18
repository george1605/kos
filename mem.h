#pragma once
#include "lib.c"
#define UPPER_MEM 0x100000
#define VM_NUM 0x80000000
#define DRIV_MEM 0xA0000000
#define LM_NUM 0x1f0000
#define KERN_MEM 2
#define USER_MEM 4
#define IO_MEM 8
#define _iom(x) (void *)(x + DRIV_MEM);
#define _pm(x) (void *)(x - VM_NUM)
#define _vm(x) (void *)(x + VM_NUM)
#define _lm(x) (void *)(x + LM_NUM)
#define PAGE_SIZE 4096

int *heapbrk = _vm(10);
int *lowbrk = _lm(10);
int fblkcnt = 0;

struct _fblock
{
  void *ptr;
  int size;
} * freeblks;

void mem_init()
{
  freeblks = (struct _fblock *)0x2FFFF00;
}

void *smalloc(int bytes)
{
  int *u = lowbrk;
  lowbrk += bytes;
  *u = 0xdeadbeef;
  *(u + bytes) = 0;
  return u;
}

void *alloc(void *start, int bytes)
{
  int *u = heapbrk;
  if (start == 0)
    heapbrk += (bytes + 2);
  else if (start > heapbrk)
    heapbrk = (int*)start + bytes + 2;
  else
    heapbrk += (bytes + 2);

  *(u + 1) = 0xdeadbeef;
  *(u + bytes) = 0;
  return (void *)(u + 2);
}

void *kcalloc(int blocks, int bytes)
{
  void *_ret = alloc(0, blocks * bytes);
  if (_ret != 0)
  {
    *(int *)_ret = 0;
    *(int *)(_ret + blocks * bytes) = 0;
  }
  return _ret;
}

struct mempage
{
  int size;
  size_t ptr;
  struct mempage *next;
  struct mempage *prev;
  struct mempage *head;
};

#define TALLOC(x) alloc(0, sizeof(x))

void *pgalloc(int bytes)
{
  int *u = heapbrk;
  heapbrk += (bytes + 2);
  *(u + 1) = 0xdead2bad;
  *(u + bytes) = 0;
  return (u + 2);
}

void pgfree(int *page)
{
  if (page != 0)
  {
    *(page - 1) = 0;
    *page = 0;
  }
}

void *kalloc(int bytes, int mode)
{
  if (mode == USER_MEM)
    return alloc(0, bytes);
  if (mode == KERN_MEM)
    return smalloc(bytes);
}

void free(int *start)
{
  int u = 0;
  if (start != 0)
  {
    while (*(start + u) != 0 && u <= 99)
    {
      *(start + u) = 0;
      u++;
    }
  }
  struct _fblock i;
  i.ptr = start;
  i.size = u;
  freeblks[++fblkcnt] = i; // adds a free block
  heapbrk -= (u + 2);
}

void freeb(char *start)
{
  int u = 0;
  if (start != 0)
  {
    while (*(start + u) != 0 && u <= 99)
    {
      *(start + u) = 0;
      u++;
    }
  }
  struct _fblock i;
  i.ptr = start;
  i.size = u;
  freeblks[++fblkcnt] = i;
  heapbrk -= (u + 2);
}

int kvalmem(int *u)
{ // checks if valid memory
  if (*(u - 1) == 0xdeadbeef || *(u - 1) == 0xdead2bad || *(u - 1) == 0xfeedc0de)
    return 1;
  return 0;
}

// if the value of the pointer has been altered
// it goes to the first block ( first after metadata )
void kfree(struct mempage *u)
{
  int *t = (int *)(u->ptr);
  if (t > 0 && kvalmem(t))
  {
    *t = 0;
    *(t - 1) = 0;
  }
  else
  {
    while (!kvalmem(t))
      t--;

    *t = 0;
    *(t - 1) = 0;
  }
}

void sfree(void *ptr)
{
  while (!kvalmem(ptr))
    ptr--;

  *(int *)ptr = 0;
}

uint8_t *maptxt()
{
  return _iom(0xb8000);
}

struct circbuf
{ // circular buffer
  char *buffer;
  size_t length;
  size_t point;
  size_t tail;
};

void initcbuf(struct circbuf *x, size_t size)
{
  x->buffer = (char*)kalloc(size, KERN_MEM);
  x->length = size;
  x->point = 0;
  x->tail = 0;
}

void pushcbuf(struct circbuf *x, char chr)
{
  if (x->point >= x->length)
    x->point %= x->length;
  else
    x->point++;

  x->buffer[x->point] = chr;
}

void erscbuf(struct circbuf *x)
{
  x->buffer[x->point] = 0;
  if (x->point == 0)
    x->point = x->length - 1;
  else
    x->point--;
}

char readcbuf(struct circbuf *x, int off)
{
  if ((x->tail + off) > x->length)
    return -1;
  return x->buffer[x->tail + off];
}

void* __alloca(size_t size)
{
  char* c = (char*)getframe();
  c -= size;
  *c = 0;
  return (void*)c;
}
