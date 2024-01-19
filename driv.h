#pragma once
#include "lib.c"
#include "smem.h"
#include "fs.h"
#include "dma.h"
#include "elf.h"
#define IOMEM_DMA 1
#define VERSION(a,b,c) a * 1000 + b * 100 + c 
#define MODULE_VERSION(str) obj->version = __make_version(str)
#define MODULE_NAME(str) obj->name = strdup(str)
int* drivbrk = (int*)(DRIV_MEM + 10);

// convert "1.0.0" -> VERSION(1, 0, 0)
int __make_version(char* str)
{
  int v1, v2, v3;
  char* p = strtok0(str, '.');
  return VERSION(v1, v2, v3);
}

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
  k(&u);
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
  k(&u);
  u.startio();
}

#define EXPORT_DRIVER(x) drlist.funcs[++drlist.cnt] = (drentry)x

void drload(char* name)
{
  ElfHeader* buffer = (ElfHeader*)kalloc(sizeof(struct ElfHeader), KERN_MEM);
  elfload_ext(name, (char*)buffer);
  EXPORT_DRIVER(buffer->e_entry);
  free(buffer);
}

void drivinit()
{
  drivobj obj;
  for(int i = 0;i < drlist.cnt;i++)
  {
    if(drlist.funcs[i] != NULL_PTR)
    {
      drlist.funcs[i](&obj);
      obj.init();
      printf("Loaded driver %s\n", obj.name);
    }   
  }
}

void drivexit()
{
  drivobj obj;
  for(int i = 0;i < drlist.cnt;i++)
  {
    if(drlist.funcs[i] != NULL_PTR)
    {
      drlist.funcs[i](&obj);
      obj.unload();
    }   
  }
  memset(drlist.funcs, 0, sizeof(drentry) * drlist.cnt);
  drlist.cnt = 0;
}

struct vfile* drivasfile(drivobj obj)
{
  struct vfile* vf = kalloc(sizeof(struct vfile));
  vf->name = obj.name;
  vf->fd = fdalloc();
  return vf; 
}