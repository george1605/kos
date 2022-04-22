#pragma once
#include "lib.c"
#undef NOP
#define NOP() asm("mov r0, r0")
#define ARM_EXIT 1 /* system calls */
#define ARM_FORK 2
#define ARM_READ 3
#define ARM_WRITE 4
#define ARM_PCI_BASE 0xfee00000
void xrqinstall(size_t ndx, void* addr)
{
    size_t *v;
    v = (size_t*)0x0;
    v[ndx] = 0xEA000000 | (((size_t*)addr - 8 - (4 * ndx)) >> 2);
}

size_t cpsrget(){
    size_t r;
    asm("mrs %[ps], cpsr" : [ps]"=r" (r));
    return r;
}

void cpsrset(size_t r)
{
    asm("msr cpsr, %[ps]" : : [ps]"r" (r));
}

#define XCHG(reg1, reg2) 

void __swi(uint8_t call)
{
    asm volatile("swi %0":"r"(call));
}

static inline void __writew(u16 val, volatile void *addr)
{
    asm volatile("strh %1, %0"
                 :
                 : "Q"(*(volatile u16 *)addr), "r"(val));
}

static inline u16 __readw(const volatile void* addr)
{
    u16 val;
    asm volatile("ldrh %0, %1"
                 : "=r"(val)
                 : "Q"(*(volatile u16*)addr));
    return val;
}

#define FORCE_REG(x, y) register uint8_t x asm(y)