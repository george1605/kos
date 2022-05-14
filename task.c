#pragma once
#define MEM_TASK 0x2
#define GUI_TASK 0x4
#define IO_TASK 0x8
#define NET_TASK 0xF
#include "process.h"

struct task {
 int flags;
 int pid;
 int cnt;
 int perm; // permissions
 char* res;
 size_t sems[8]; // sempahores
 int(*f)(void);
};

struct task newtask(int type){
  struct task t;
  struct proc u = prcreat("NEW TASK");
  t.pid = u.pid;
  t.flags = type;
  if(type & MEM_TASK)
     t.res = u.stack;
  return t;
}

void endtask(struct task u){
  HALT();
  if(u.flags & MEM_TASK)
    free((int*)u.res);
  else
    u.res = (char*)0;
  u.pid = 0;
}

struct task duptask(struct task i){
  struct task u;
  struct proc p;
  u.flags = i.flags;
  return u;
}

void pushsem(struct task i, size_t value){
  if(i.cnt < 8)
    i.sems[++i.cnt] = value;
}

void exectask(struct task i){
  i.sems[0] = 0;
  i.f();
}

struct task creatask(int var, void* ret, int(*myf)(void)){
   struct task i = newtask(MEM_TASK | IO_TASK);
   i.f = myf;
   i.sems[0] = var;
   *(int*)ret = myf();
   return i;
}

void loadtask(){
  int __ignore;
  asm("mov %0, %%ax; ltr %ax":"=r"(__ignore):"r"(0x28));
}

struct ptask 
{
  int priot;
  int pid;
  int flags;
  void(*f)();
  void* ctx;
} *ptasks;

struct appinfo
{
  int pid;
  int ppid;
  long memusg;
  size_t instr;
};

void* proc_alloc(struct proc n, int size, struct appinfo* inf)
{
  if(n.state == KILLED || inf == NULL_PTR)
    return;
  void *x = alloc(0, size);
  inf->memusg += size;
  inf->pid = n.pid;
  return x;
}

void proc_free(void* mem, int size, struct appinfo* inf)
{
  if(*(int*)(mem + size) != 0)
    return;
  inf->memusg -= size;
}

void* __alloc(int size)
{
  struct appinfo t = {0};
  return proc_alloc(tproc, size, &t);
}

void __free(void* x)
{
  int n = strlen(x);
  struct appinfo t = {0};
  proc_free(x, n, &t);
}

#ifdef __ARM__
void pload()
{
  asm volatile("STMFD  r13!, {r0-r5}");
}

#endif

void psched()
{
  int a, max = -100;
  for(a = 0;ptasks[a].pid != 0;a++)
  {
    if(ptasks[a].priot > max)
    { 
      max = ptasks[a].priot;
      ptasks[a].f();
    }
  }
}