#pragma once
#include "lib.c"
#include "port.h"
#include "mem.h"

#define LID               0x0020/4
#define ICRHI            (0x0310/4)   // Interrupt Command [63:32]
#define ICRLO            (0x0300/4)   // Interrupt Command
#define TIMER            (0x0320/4)   // Local Vector Table 0 (TIMER)
#define LAPIC_INIT       0x00000500 
#define LAPIC_STARTUP    0x00000600
#define LAPIC_ASSERT     0x00004000   // Assert interrupt (vs deassert)
#define LAPIC_DEASSERT   0x00000000
#define LAPIC_LEVEL      0x00008000 
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
volatile uint32_t *lapic;
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

void ioapic_init(void)
{
  int i, id, maxintr;

  ioapic = (volatile struct ioapic*)IOAPIC;
  maxintr = (ioapicread(REG_VER) >> 16) & 0xFF;
  id = ioapicread(REG_ID) >> 24;
  if(id != ioapicid)
    kprint("ioapic: id isn't equal to ioapicid!\n");

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

static void lapicw(int index, int value)
{
  lapic[index] = value;
  (void)lapic[LID];  // wait for write to finish, by reading
}

void lapic_send_ipi(int i, uint32_t val) {
	lapicw(0x310, i << 24);
	lapicw(0x300, val);
	do { asm volatile ("pause" : : : "memory"); } while (lapic[0x300] & (1 << 12));
}

void lapic_startup(uint8_t apicid, uint32_t addr)
{
  int i;
  uint16_t *wrv;
  outb(0x70, 0xF);  // offset 0xF is shutdown code
  outb(0x71, 0x0A);
  wrv = (uint16_t*)_vm((0x40<<4 | 0x67));  // Warm reset vector
  wrv[0] = 0;
  wrv[1] = addr >> 4;

  lapicw(ICRHI, apicid<<24);
  lapicw(ICRLO, LAPIC_INIT | LAPIC_LEVEL | LAPIC_ASSERT);
  microdelay(200);
  lapicw(ICRLO, LAPIC_INIT | LAPIC_LEVEL);
  microdelay(100);    // should be 10ms, but too slow in Bochs!

  for(i = 0; i < 2; i++){
    lapicw(ICRHI, apicid<<24);
    lapicw(ICRLO, LAPIC_STARTUP | (addr>>12));
    microdelay(200);
  }
}

void lapic_wakeup()
{
  lapic_send_ipi(0, 0x7E | (3 << 18));
}

void lapic_tlb_shootdown(uint64_t vaddr) {
  if (!lapic) return;
  lapic_send_ipi(0, 0x7C | (3 << 18));
}