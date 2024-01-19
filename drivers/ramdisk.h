/***
RamDisk - an alternative to Physical Drives
***/
#pragma once
#include "../smem.h"
#include "../vfs.h"
#include "../process.h"
#include "../fs.h"
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

uint8_t __ram_read(char* path, char* buffer, ext2_gen_device* dev, void* priv)
{
  struct ramdisk* disk = (struct ramdisk*)priv;
  strcpy(buffer, (char*)disk->ptr);
  return 0;
}

uint8_t __ram_write(char* path, char* buffer, uint32_t len, ext2_gen_device* dev, void* priv)
{
  struct ramdisk* disk = (struct ramdisk*)priv;
  if(!(disk->prot & PAGE_WRITABLE)) return 0xFF;
  size_t sz = min(disk->size, len);
  memcpy(disk->ptr, buffer, sz);
  return 0;
}

uint8_t __ram_read_dir(char* dir, char *, ext2_gen_device *, void *)
{

}

void rammount(struct ramdisk* disk, char* name)
{
  filesystem* ops = (filesystem*)kalloc(sizeof(filesystem), KERN_MEM);
  ops->name = "RAMFS";
  ops->priv_data = (uint8_t*)disk;
  ops->read = __ram_read;
  ops->writefile = __ram_write;
  ops->read_dir = __ram_read_dir;
  mount(name, ops);
}

void ramunmount(char* name)
{
  unmount(name);
}

void ramunlink(struct ramdisk* i)
{
 i->fd = -1;
 i->ptr = NULL_PTR; 
}

void ramsetup()
{
  atadev dev;
  ramdisk* disk;
  ata_device_detect(&dev, 1);
  if(ataerr == ATA_NOTEXIST)
  {
    disk = (ramdisk*)kalloc(sizeof(ramdisk), KERN_MEM);
    disk->ptr = kalloc(4096, KERN_MEM);
    disk->size = 4096;
    disk->prot = PAGE_PRESENT | PAGE_WRITABLE;
    rammount(disk, "/home/disk0");
  }
}

void ramexit()
{
  int id = 0;
  for(int i = 0;i < MAX_MOUNTS;i++)
    if(!strcmp(mount_points[i].name, "/home/disk0"))
      {
        id = i;
        break;
      }
  struct ramdisk* disk = (struct ramdisk*)mount_points[id].dev->fs->priv_data;
  free(disk->ptr);
  free(disk);
}