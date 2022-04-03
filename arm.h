#pragma once
#include "lib.c"
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

void xchgax(){
    asm("xchg ax,bx");
}

void mulr(){
    asm("mul R1,R2,R3"); 
}

void xchgbx(){
    asm("xchg bx,cx");
}

void xchcx(){
    asm("xchg cx,dx");   
}
