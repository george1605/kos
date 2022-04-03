#pragma once
#include "lib.c"
#include "mem.h"
#define USE_LOWMEM 1
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