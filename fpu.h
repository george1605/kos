#pragma once
#include "lib.c"

void fpu_set(const uint16_t cw)
{
    asm volatile("fldcw %0" ::"m"(cw));
}

void sse_init()
{ // just settin' the CRs
    size_t CR0;
    asm("mov %%cr0, %0"
        : "=r"(CR0));
    CR0 &= ~(1 << 2);
    CR0 |= (1 << 1);
    asm("mov %0, %%cr0" ::"r"(CR0));
    asm("mov %%cr4, %0"
        : "=r"(CR0));
    CR0 |= (1 << 9);
    CR0 |= (1 << 10);
    asm("mov %0, %%cr4" ::"r"(CR0));
}

void fpu_init()
{
    unsigned int t;

    asm("clts");
    asm("mov %%cr0, %0"
        : "=r"(t));
    t &= ~(1 << 2);
    t |= (1 << 1);
    asm("mov %0, %%cr0" ::"r"(t));
    asm("mov %%cr4, %0"
        : "=r"(t));
    t |= 3 << 9;
    asm("mov %0, %%cr4" ::"r"(t));
    asm("fninit");
}
