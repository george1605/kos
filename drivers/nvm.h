#pragma once
#include "../port.h"
#include "../pci.h"
#include "../lib.c"
#define NVMREAD 0x02
#define NVMWRITE 0x01
#define NVMIDENT 0x06
#define CAPMAX 44

enum BAR0_REG
{
  CAP = 0x0,
  VS = 0x8,
  INTMS = 0xC,
  INTMC = 0x10,
  CC = 0x14,
  CSTS = 0x1F,
  AQA = 0x24,
  ASQ = 0x28,
  ACQ = 0x30
};

struct nvmqueue 
{
  void* info;
  int size;
};

struct nvmdev
{
  struct pcidev pci;
  struct spinlock lock;
};

void nvmalloc(struct nvmdev i)
{
  initlock(&i.lock, "NVMe");
  i.pci.pclass = 1;
}

void nvmlock(struct nvmdev i)
{
  acquire(&i.lock);
}

void nvmunlock(struct nvmdev i)
{
  release(&i.lock);
}

size_t nvmread(struct nvmdev i)
{
  if (!i.lock.locked)
    return (size_t)pciread(0, 0, NVMREAD, 0);
  return 0;
}

struct nvmdev nvmsetup()
{
  struct nvmdev u;
  u.pci.pclass = 1;
  u.pci.subclass = 8;
  u.pci.progif = 2;
  return i;
}