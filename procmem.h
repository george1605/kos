#include "process.h"
#include "mem.h"

struct kmem {
  int pid;
  char* addr;
  int size;
};


void contclr(struct context u){
  u.edi = 0;
  u.esi = 0;
  u.ebp = 0;
  u.ebx = 0;
  u.eip = 0;
}

void contswap(struct context _con){
  ccon = _con;
}

void pracc(struct proc pr,struct kmem u){
  if(u.pid == 0)
    u.pid = pr.pid;
}

void prdacc(struct kmem u){
   u.pid = 0;
}

void kmfree(struct kmem u){
  free(u.addr);
  u.pid = 0;
  u.size = 0;
}
