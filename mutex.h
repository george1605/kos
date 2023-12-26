#pragma once
#include "pit.c"
#include "fs.h"
#include "process.h"

struct thread {
  int tid;
  prstate state;
  func f;
  char* stack;
  int ssize;
  struct file* ofiles;
} cthread;
typedef struct thread* thpool;

void schedule(struct thread x, thpool pool)
{
    int a = 0;
    while(pool[a].state != KILLED) a++;
    pool[a] = x;
    x.f(0,(char**)0);
}

struct ioctx {
  struct thread thr;
  int ports[3];
};

void iolisten(int port){
  while(!inb(port));
}


struct thread thnew(){ /* just creates a process */
  struct proc k;
  struct thread i;
  prinit(k,0);
  i.tid = k.pid + 1;
  i.state = STARTED;
  i.stack = k.stack;
  return i;
}

struct thread thcreat(struct proc* parent,int sstack){
  struct thread u;
  u.tid = parent->pid + 1;
  u.state = STARTED;
  u.stack = parent->stack;
  if(sstack < parent.ssize)
    u.ssize = sstack;
  else   
    u.ssize = parent->ssize;
}

void thpause(struct thread t){
  HALT();
  t.state = PAUSED;
}

void thrun(struct thread k){
  t.state = STARTED;
  t.f(0,(char**)0);
}

void thrunp(struct thread a,struct thread b){
  if(timer_ticks % 2 == 0){
    thpause(b);
    thrun(a);
  }else{
    thpause(a);
    thrun(b);
  }
}

struct mutex {
  int tid;
  char* mem;
  int size;
  int lock;
};

struct fitex {
  int tid;
  int fd;
  int lock;
};

void mutex_lock(struct mutex u){
  u.lock = 1;
}

void thexit(){
  cthread.state = KILLED;
  int c = 0;
  while(cthread.ofiles[c].open == 1)
    fclose(&cthread.ofiles[c]), c++;
    
}

void lockinc()
{
  asm volatile(
    "lock;"
    "incl (%eax)"
  );
}