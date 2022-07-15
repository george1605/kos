#pragma once
#include "lib.c"
#include "mem.h"
#define USE_LOWMEM 1
#define SPARC_PAGE 8192
int* spbreak = 0x2f000; 

void clearg0(){
  asm("clr %g0");
}

void compg0(int value){
  int p;
  asm("clr %g0,%1":"=r"(p),"r"(value));
}

void* sparc_alloc(int bytes){
  int* u = spbreak;
  spbreak += bytes;
  *(u + bytes) = 0;
  return u;  
}

void sparc_free(void* ptr){
  if(ptr == 0) return;
  
  int p = strlen((char*)ptr);
  memset(ptr,0,p);
}

void sparc_out(){
  // mmio or smth
}

struct SparcRegs
{
  uint16_t i0, i1, i2, i3, i4;
  uint16_t o0, o1, o2, o3, o4;
  uint32_t g0, g1, g2, g3, g4;
  unit32_t l0, l1, l2, l3, l4;
};