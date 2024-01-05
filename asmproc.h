#pragma once
#include "task.c"

void prpush(int x)
{
  int __ignore;
  asm("push":"=r"(__ignore),"r"(x));
}

void* protmode(struct proc* u)
{
  if(u == NULL_PTR) return;
  void* x = prsave(*u); // for switching between real and (virtual) "protected"
  CLI();
  u->stack = (char*)alloc(0, 64000);
  u->ssize = 64000;
  asm("mov %0,%%esp": "r"(u->stack));
  return x;
}

void protend(void* n, struct proc u)
{
  fxrestor(n);
  free((int *)n);
  STI();
}

void protexec(struct proc* u)
{
  void* x = protmode(u);
  prswap(*u);
  protend(x, *u);
}