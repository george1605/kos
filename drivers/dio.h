/***
Direct IO - Avoiding the Syscalls
***/
#pragma once
#include "ramdisk.h"
#include "../port.h"
#include "../time.c"
#define DIO_VENDOR_ID  44
#define DIO_MODULE_ID  120
#define LBA28_LIMIT (1 << 28)

#define DIO_BASE       0x600000       
#define DIO_END        0x1000000       
#define DIO_DEVSIZE    0x10000         

#define DIOII_BASE     0x01000000      
#define DIOII_END      0x20000000     
#define DIOII_DEVSIZE  0x00400000    

struct diores {
  char* name;
  long start;
  long end;
};

struct diodev {
  struct diores res[3];
  struct spinlock lock;
};

void diolock(struct diodev u){
  acquire(&u.lock);
}

void diounlock(struct diodev u){
  acquire(&u.lock);
}

void lba28_read(int drive, long long addr)
{
  outb(0x1F1, 0x00);
  outb(0x1F2, 0x01);
  outb(0x1F3, (unsigned char)addr);
  outb(0x1F4, (unsigned char)(addr >> 8);
  outb(0x1F5, (unsigned char)(addr >> 16);
  outb(0x1F6, 0xE0 | (drive << 4) | ((addr >> 24) & 0x0F));
}
       
void lba48_read(int drive, long long addr)
{
   outb(0x1F1, 0x00); outb(0x1F1, 0x00);
   outb(0x1F2, 0x00); outb(0x1F2, 0x01);
   outb(0x1F3, (unsigned char)(addr >> 24));      
   outb(0x1F3, (unsigned char)addr);
   outb(0x1F4, (unsigned char)(addr >> 32));
   outb(0x1F4, (unsigned char)(addr >> 8));
   outb(0x1F5, (unsigned char)(addr >> 40));
   outb(0x1F5, (unsigned char)(addr >> 16));
   outb(0x1F6, 0x40 | (drive << 4));
   outb(0x1F7, 0x24);
}
// 
void disk_read(int drive, long long addr, void* memory)
{
  if(addr < LBA28_LIMIT)
    lba28_read(drive, addr);
  else
    lba48_read(drive, addr);
  
    while (!(inb(0x1F7) & 0x08)) {}
    size_t tmpword;
    for (idx = 0; idx < 256; idx++)
    {
        tmpword = inw(0x1F0);
        memory[idx * 2] = (unsigned char)tmpword;
        memory[idx * 2 + 1] = (unsigned char)(tmpword >> 8);
    }
}
       
int disk_detect(int disk)
{
   int p = (disk == 1)? 0x1F6 : 0x176;
   outb(p, 0xA0); 
   sleep(1); 
   tmpword = inb(0x1F7); 
   return (tmpword & 0x40);
}
       
void disk_read2(int drive, long long addr, void* memory)
{
    if(!disk_detect(drive))
      return; // in future i will use the ram disk
    disk_read(drive, addr, memory);
}
