/***
RamDisk - an alternative to Physical Drives
***/
#pragma once
#include "../smem.h"
#include "../vfs.h"
#include "../process.h"
#define WORD(a, b) ((a << 8) | b)

struct ramdisk
{
  void* ptr;
  size_t size;
  int fd;
  int prot;
};

void raminit(struct ramdisk* i)
{
  if(i == NULL_PTR)
    return;
  i->size = 4096;
  i->ptr = alloc(0, 4096);
  i->prot = M_BOTH;
}

void ramvirt(struct ramdisk* disk, char boot[512])
{
  if(WORD(boot[510], boot[511]) != 0xAA55 && WORD(boot[510], boot[511]) != 0x55AA)
    return; // not a bootable device
  disk->fd = fdalloc();
  disk->ptr = vmap(NULL_PTR, 4096, PAGE_PRESENT | PAGE_WRITABLE, (struct vfile*)NULL_PTR);
  disk->size = 4096;
  disk->prot = PAGE_PRESENT | PAGE_WRITABLE;
  sysvf[disk->fd].mem = disk->ptr; // add it to the system virtual file list
  memcpy(disk->ptr, boot, 512);
}

void ramstartcpu(struct ramdisk* disk, int cpuid)
{
  lapic_startup(cpuid, (uint32_t)disk->ptr);
}

void rammount(struct ramdisk* i, char* x)
{
  struct vfile n = vfsmap(x, i->ptr);
  i->fd = n.fd;
}

void ramunlink(struct ramdisk* i)
{
 i->fd = -1;
 i->ptr = NULL_PTR; 
}
