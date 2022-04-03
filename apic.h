#pragma once
#include "lib.c"
#include "port.h"
#define LOCAL_APIC_BASE 0xFFFFFFFFFF000

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
