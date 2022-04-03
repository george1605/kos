#pragma once
#include "lib.c"
#include "port.h"

#define LID   0x0020/4 
#ifndef T_IRQ0
  #define T_IRQ0   32  
#endif
#define IOAPIC  0xFEC00000

#define REG_ID     0x00
#define REG_VER    0x01
#define REG_TABLE  0x10

#define INT_DISABLED   0x00010000  // Interrupt disabled
#define INT_LEVEL      0x00008000  // Level-triggered (vs edge-)
#define INT_ACTIVELOW  0x00002000  // Active low (vs high)
#define INT_LOGICAL    0x00000800  // Destination is CPU id (vs APIC ID)
#define IOAPIC_REDTBL(n)   (0x10 + 2 * n) 

struct ioapic {
  size_t reg;
  size_t pad[3];
  size_t data;
};

volatile struct ioapic *ioapic;
volatile size_t *lapic;
size_t ioapicid;

static size_t ioapicread(int reg)
{
  ioapic->reg = reg;
  return ioapic->data;
}

static void ioapicwrite(int reg, size_t data)
{
  ioapic->reg = reg;
  ioapic->data = data;
}

void ioapicinit(void)
{
  int i, id, maxintr;

  ioapic = (volatile struct ioapic*)IOAPIC;
  maxintr = (ioapicread(REG_VER) >> 16) & 0xFF;
  id = ioapicread(REG_ID) >> 24;
  if(id != ioapicid)
    kprint("ioapicinit: id isn't equal to ioapicid!\n");

  for(i = 0; i <= maxintr; i++){
    ioapicwrite(REG_TABLE+2*i, INT_DISABLED | (T_IRQ0 + i));
    ioapicwrite(REG_TABLE+2*i+1, 0);
  }
}

void ioapicenable(int irq, int cpunum)
{
  ioapicwrite(REG_TABLE+2*irq, T_IRQ0 + irq);
  ioapicwrite(REG_TABLE+2*irq+1, cpunum << 24);
}


void lapicinit(){
   outb(0x22, (short)0x70);   // Select IMCR
   outb(0x23, inb(0x23) | 1);  
}

int lapicid()
{
  if (!lapic)
    return 0;

  return lapic[LID] >> 24;
}