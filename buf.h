#pragma once
#include "lib.c"
#define B_VALID 0x2
#define B_DIRTY 0x4
#define B_NONE 0x20
#define BSIZE 512
#define FSSIZE 20000

struct buf {
  int flags;
  size_t dev;
  size_t blockno;
  size_t refcnt;
  struct buf *prev;
  struct buf *next;
  struct buf *qnext;
  size_t off;
  uint8_t data[BSIZE];
};

struct sbuf {
  uint8_t* data;
  size_t size;
  size_t off; //
  int flags;
  size_t dev;
};

struct buflist {
  struct buf** buffs;
  int cnt;
};

struct buf* bset(struct buf* u,int flg){
  if((flg & 0x2) || (flg & 0x4))
    u->flags = flg;
  
  return u;
}

struct buf* bcreat(size_t blockno, struct buf* prev)
{
  struct buf* u;
  u->blockno = blockno;
  if(prev != 0){
   u->prev = prev;
   u->dev = prev->dev;
  }
  u->data[0] = 0;
  return u;
}

struct buf* bnext(struct buf* u){
  if(u->next == 0)
    return u->qnext;
  else
    return u->next;
}

struct buf* bclr(struct buf* u){
  int t = 0;
  while(u->data[t] != 0){
    u->data[t] = 0;
    t++;
  }
}

int bcasti(struct buf* u, int off){ //casts the bytes to an int using POINTERS!!!
  int* ptr = (int*)(u->data + off);
  return (*ptr);
}

short bcastw(struct buf* u, int off){
  short* ptr = (short*)(u->data + off);
  return (*ptr);
}

long bcastl(struct buf* u, int off){
  long* ptr = (long*)(u->data + off);
  return (*ptr);
}
//defined in ide.c
void bread(struct buf* b,int disk);
int bwrite(struct buf* b,int disk);
