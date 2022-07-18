/***
RamDisk - an alternative to Physical Drives
***/
#include "../smem.h"
#include "../vfs.h"

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
