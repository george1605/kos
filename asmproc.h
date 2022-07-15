#pragma once
#include "procmem.h"
#include "task.c"

void __pasm(int argc,char** argv)
{
  if(argc == 1)
   asm(argv[0]);
}

void prasm(struct proc u,const char* u)
{
  u.f = __pasm;
  char* t[] = {u};
  if(u.stack == (char*)0)
    u.stack = alloc(0,64);
  prswap(u);
  prexec(1,(char**)t);
}

void prpush(int x)
{
  int __ignore;
  asm("push":"=r"(__ignore),"r"(x));
}

void* protmode(struct proc u)
{
  void* x = prsave(u); // for switching between real and protected
  CLI();
  u.stack = alloc(0, 64000);
  u.ssize = 64000;
  asm("mov %0,%%esp": "r"(u.stack));
  return x;
}

void protend(void* n, struct proc u)
{
  fxrestor(n);
  free((int *)n);
  STI();
}

void protexec(struct proc u)
{
  protmode(u);
  prswap(u);
  protend(u);
}