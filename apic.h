#pragma once
#include "lib.c"
#include "port.h"
#include "irq.c"
#define LOCAL_APIC_BASE 0xFFFFFFFFFF000

long apicbase;
long apicvirt;

#define APIC_READ(off) mminq(apicvirt + off)
#define APIC_WRITE(off, data) mmoutq(apicvirt + off, data)

uint64_t apicreadb()
{
    uint64_t low;
    uint64_t high;
    asm("rdmsr"
        : "=a"(low), "=d"(high)
        : "c"(0x1B));

    return (high << 32) | low;
}

void apicwriteb(uint64_t val)
{
    uint64_t low = val & 0xFFFFFFFF;
    uint64_t high = val >> 32;
    asm("wrmsr" ::"a"(low), "d"(high), "c"(0x1B));
}

void apicenable()
{
    apicwriteb(apicreadb() | (1UL << 11));
    outb(0xF0, inb(0xF0) | 0x1FF);
}

void apichnd(struct regs* r)
{
    asm("nop");
}

void apicinit()
{
    apicbase = apicreadb() & LOCAL_APIC_BASE;
    apicvirt = iomapping(apicbase);
    idt_set_gate(0xFF, (unsigned)apichnd, 0x08, 0x8E);
    apicenable();
}
