#pragma once
#include "lib.c"
#include "smem.h"
#include "fs.h"
#include "dma.h"
#define IOMEM_DMA 1

int* drivbrk = (int*)(DRIV_MEM + 10);

void dbgprint(char* u){
  kprint(u);
}

void dbgerr(char* u){
  perror(u);
}

struct iomem 
{
  size_t channel; // for DMA
  size_t size; // also for DMA
};

// similar to iorequest()
void* iommap(void* mem, struct iomem info, int type)
{
  dmablock blk;
  if(type == IOMEM_DMA)
  {
    dmafrom(&blk, (char*)mem);
    blk.length = info.size;
    dmastart(info.channel, &blk, 0);
    return vmap(mem, info.size, 0, (struct vfile*)NULL_PTR); // maps to virtual memory
  }
  return NULL_PTR;
}

struct drivobj {
  char* name;
  int version;
  short flags;
  void(*init)();
  void(*startio)();
  void(*unload)();
};

struct driv // more like linux modules
{
  void(*mod_init)();
  void(*mod_exit)();
};

typedef void(*drentry)(struct drivobj* u); //just setup the drivobj

struct drivlist
{
  int cnt;
  drentry* funcs;
} drlist;

void* dralloc(drentry k){
  int* u = drivbrk;
  drivbrk += 6;
  *u = 0xFEEDC0DE;
  *(u + 1) = (int)k;
  return (void)(u + 1);
}

void drexec(drentry k){
  struct drivobj u;
  u.flags = 0;
  u.version = 1;
  k(u);
  u.init();
}

void drrun(void* u){
  if(u != 0 && *u != 0 && *(u-1) == 0xFEEDC0DE){
   drentry k = *(drentry*)(u);
   drexec(k);
  }
}

void drfree(void* u){
  int* t = u;
  if(t != 0){
    *t = 0;
    *(t-1) = 0;
  }
}

void drstartio(drentry k){
  struct drivobj u;
  k(u);
  u.startio();
}

#define EXPORT_DRIVER(x) drlist.funcs[++drlist.cnt] = (drentry)x

void drload(struct file f)
{
  char* u = ata_read_sector(&ata_primary_master, f.fd);
  EXPORT_DRIVER(u);
}