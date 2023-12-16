#pragma once
#include "port.h"
#include "mem.h"
#define MPPROC    0x00
#define MPBUS     0x01
#define MPIOAPIC  0x02
#define MPIOINTR  0x03
#define MPLINTR   0x04

struct mp
{                     
  uint8_t sign[4]; 
  void *physaddr;     
  uint8_t length;       // 1
  uint8_t specrev;      // [14]
  uint8_t checksum;     // all bytes must add up to 0
  uint8_t type;         // MP system config type
  uint8_t imcrp;
  uint8_t reserved[3];
};

struct mpioapic 
{
  uint8_t type; 
  uint8_t id;
  uint8_t version;
  uint8_t flags; 
  uint32_t address;
};

struct mp* mpsearchx(size_t a, int len)
{
  uint8_t *e, *p, *addr;

  addr = _vm(a);
  e = addr + len;
  for (p = addr; p < e; p += sizeof(struct mp))
    if (memcmp(p, "_MP_", 4) == 0 && sum(p, sizeof(struct mp)) == 0)
      return (struct mp *)p;
  return 0;
}

struct mp* mpsearch(){
  uint8_t *bda;
  uint p;
  struct mp *mp;

  bda = (uint8_t *)_vm(0x400);
  if ((p = ((bda[0x0F] << 8) | bda[0x0E]) << 4)) // address 0x800040F
  {
    if ((mp = mpsearchx(p, 1024)))
      return mp;
  }
  else
  {
    p = ((bda[0x14] << 8) | bda[0x13]) * 1024;
    if ((mp = mpsearchx(p - 1024, 1024)))
      return mp;
  }
  return mpsearchx(0xF0000, 0x10000);
}

void mpinit(){
  outb(0x22, 0x70);   // Select IMCR
  outb(0x23, inb(0x23) | 1);
}