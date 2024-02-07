#pragma once
#include "lib.c"
#include "smem.h"
#include "fs.h"
#include "dma.h"
#include "elf.h"
#include "drivers/ioctl.h"
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

struct kmodule
{
  char* name;
  char ioctl_code; // a code ranging from
  void(*entry)();
  void(*exit)();
  void(*ioctl)(int, size_t, size_t);
};

void kmod_load(struct kmodule* module)
{
  struct mod_info info;
  info.name = module->name;
  info.entry = module->entry;
  info.exit = module->exit; 
  mod_load(&info);
  hashmap_set(&ioctlmap.i_map, (void*)module->ioctl_code, module->ioctl);
}

void kmod_unload(char* name)
{
  mod_unload(name);
}

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