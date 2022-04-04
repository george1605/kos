#pragma once
#include "procmem.h"

void __pasm(int argc,char** argv){
  if(argc == 1)
   asm(argv[0]);
}

void prasm(struct proc u,const char* u){
  u.f = __pasm;
  char* t[] = {u};
  if(u.stack == (char*)0)
    u.stack = alloc(0,64);
  prswap(u);
  prexec(1,(char**)t);
}

void prpush(int x){
  int __ignore;
  asm("push":"=r"(__ignore),"r"(x));
}

void protmode(struct proc u){
  prsave(u); // for switching between real and protected
  CLI();
  u.stack = malloc(0, 64000);
  u.ssize = 64000;
  asm("mov %0,%%esp": "r"(u.stack));
}
