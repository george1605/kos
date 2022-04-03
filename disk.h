#pragma once
#include "port.h"
#include "ata.h"
#define SECTSIZE 512
#define DISK_PORT 0x1f0
#define DISKP(n) DISK_PORT + (n % 7)

struct diskfree
{ 
  size_t tclusters;
  size_t aclusters;
  size_t spc;
  size_t bps;
};

long long abytes(struct diskfree u){
  return (u.aclusters * u.spc * u.bps);
}

void waitdisk(void)
{
  while((inb(0x1F7) & 0xC0) != 0x40);
}

void readsect(void *dst,int offset)
{

  waitdisk();
  outb(0x1F2, 1);   // count = 1
  outb(0x1F3, offset);
  outb(0x1F4, offset >> 8);
  outb(0x1F5, offset >> 16);
  outb(0x1F6, (offset >> 24) | 0xE0);
  outb(0x1F7, 0x20);  // cmd 0x20 - read sectors

  waitdisk();
  insl(0x1F0, dst, SECTSIZE/4);
}

void readseg(char* pa, uint32_t count, uint32_t offset)
{
  char* epa;
  epa = pa + count;

  pa -= offset % SECTSIZE;
  offset = (offset / SECTSIZE) + 1;
  for(; pa < epa; pa += SECTSIZE, offset++)
    readsect(pa, offset);
}

void readboot(){
 char* u = (char*)0x100000;
 readseg(u,512,0);
 kprint(u);
}
