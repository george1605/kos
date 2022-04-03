#pragma once
#include "irq.c"
#include "buf.h"
#include "process.h"
#ifndef SECTOR_SIZE
  #define SECTOR_SIZE   512
#endif
#define IDE_BSY       0x80
#define IDE_DRDY      0x40
#define IDE_DF        0x20
#define IDE_ERR       0x01

#define IDE_CMD_READ  0x20
#define IDE_CMD_WRITE 0x30
#define IDE_CMD_RDMUL 0xc4
#define IDE_CMD_WRMUL 0xc5
typedef unsigned long long daddr_t;//disk address

static int havedisk1;
daddr_t diskbrk = 512 * 0x1F;
static struct buf *idequeue;
struct proc diskmgr;

static int idewait(int checkerr)
{
  int r;

  while(((r = inb(0x1f7)) & (IDE_BSY|IDE_DRDY)) != IDE_DRDY);
  if(checkerr && (r & (IDE_DF|IDE_ERR)) != 0)
    return (-1);
  return 0;
}

static void ideinit(){
  idewait(0);
  int i;
  diskmgr = prcreat("Disk Manager");

  outb(0x1f6, 0xe0 | (1<<4));
  for(i=0; i<1000; i++){
    if(inb(0x1f7) != 0){
      havedisk1 = 1;
      break;
    }
  }

  outb(0x1f6, 0xe0 | (0<<4));
}


static void idestart(struct buf *b)
{
  if(b == 0)
    return;
  if(b->blockno >= FSSIZE)
    return;
  int sector_per_block =  BSIZE/SECTOR_SIZE;
  int sector = b->blockno * sector_per_block;
  int read_cmd = (sector_per_block == 1) ? IDE_CMD_READ :  IDE_CMD_RDMUL;
  int write_cmd = (sector_per_block == 1) ? IDE_CMD_WRITE : IDE_CMD_WRMUL;

  if (sector_per_block > 7) return;

  idewait(0);
  outb(0x3f6, 0);
  outb(0x1f2, sector_per_block);
  outb(0x1f3, sector & 0xff);
  outb(0x1f4, (sector >> 8) & 0xff);
  outb(0x1f5, (sector >> 16) & 0xff);
  outb(0x1f6, 0xe0 | ((b->dev&1)<<4) | ((sector>>24)&0x0f));
  if(b->flags & B_DIRTY){
    outb(0x1f7, write_cmd);
    outsl(0x1f0, b->data, BSIZE/4);
  } else {
    outb(0x1f7, read_cmd);
  }
}

void ideintr(struct regs* r)
{
  struct buf *b;

  if((b = idequeue) == 0){
    return;
  }
  idequeue = b->qnext;

  if(!(b->flags & B_DIRTY) && idewait(1) >= 0)
    insl(0x1f0, b->data, BSIZE/4);

  b->flags |= B_VALID;
  b->flags &= ~B_DIRTY;
}

void idenew(){
  irq_install_handler(14,ideintr);
}

void iderw(struct buf* b){
  idewait(0);
  if(b->dev > 0xFF)
    return;
  else
    idestart(b);
}

struct buf* bget(size_t dev,size_t blockno){
  idewait(0);
  struct buf* t;
  t->blockno = blockno;
  t->dev = dev;
  t->next = (struct buf*)0;
  return t;
}

void bread(struct buf* b,int disk){
  if(b == 0)
    return;

  if((b->flags & B_VALID) == 0) {
    iderw(b);
  }
}

struct buf* breads(int dev,int blockno){
  struct buf* b = TALLOC(struct buf);
  b->dev = dev;
  b->blockno = blockno;
  bread(b,b->dev);
  return b;
}

void brelse(struct buf* b){
   free(b);
}

int bwrite(struct buf* b,int disk){
  b->flags |= B_DIRTY;
  iderw(b);
}

void bappend(struct buf* parent, struct buf* u){
  if(parent != 0 && u != 0)
   parent->qnext = u;
}

int dalloc(int bytes){
  int k = diskbrk;
  diskbrk += bytes;
  return k;
}

void read_sect(size_t _Sector){
  struct buf* x = (struct buf*)0x3A0000;
  x->blockno = 2;
  x->flags = B_VALID;
  x->next = 0;
  x->dev = 0;
  bread(x,_Sector);
  kprint((char*)x->data);
}

struct farea {
  struct buf* buffer;
  struct buf* head;
  size_t size;
  size_t offset;
};

#define BUFSET(b, off, val) write32((void*)(b->data + off), val)
#define BUFGET(b, off) read32((void *)(b->data + off))

struct farea* falloc(int sect, int dev, long len){
 struct farea* f = TALLOC(struct farea);
 f->offset = 0;
 f->size = len;
 struct buf* b = TALLOC(struct buf);
 BUFSET(b,0,0xdeadbeef);
 b->flags = B_DIRTY;
 b->blockno = sect;
 if(len < 512){
   f->buffer = b;
 }else{

 }
 bwrite(b, dev);
 return b;
}

#define SCT_RESERVED 0x20 // reserved section
#define SCT_USER 0x21

struct disksct {
  int diskno;
  int flags;
  long long start;
  long long end;
};

struct disksct sctalloc(int dev, long long start){
  struct buf* x = kalloc(sizeof(struct buf),KERN_MEM);
  memset(x->data,0,512);
  x->blockno = start/512 - 1;
  bwrite(x,dev);
  struct disksct u;
  u.start = start;
  u.end = start + 1024;

}

void sctfree(struct disksct u){
  u.start = 0;
  u.end = 0;
}