#pragma once
#define MEM_TASK 0x2
#define GUI_TASK 0x4
#define IO_TASK 0x8
#define NET_TASK 0xF
#include "process.h"
typedef struct spinlock semaphore;

struct task {
 int flags;
 int pid;
 int perm; // permissions
 char* stack;
 size_t size;
 struct proc* parent;
 struct file* ofiles;
 jmp_buf ctx;
 int(*f)(void);
};

typedef struct {
  struct task** threads;
  int num_threads;
  int current_thread;
} scheduler;

void loadtask(){
  int __ignore;
  asm("mov %0, %%ax; ltr %ax":"=r"(__ignore):"r"(24));
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

int subproc_check(scheduler* scheduler, int thread_id) {
  if (setjmp(scheduler->threads[thread_id]->ctx) == 0) {
      longjmp(scheduler->threads[scheduler->current_thread]->ctx, 1);
  }
}

// a task switcher inside a process
void subproc_schedule(scheduler* scheduler) {
    if (setjmp(scheduler->threads[scheduler->current_thread]->ctx) == 0) {
        // Switch to the next thread
        scheduler->current_thread = (scheduler->current_thread + 1) % scheduler->num_threads;
        longjmp(scheduler->threads[scheduler->current_thread]->ctx, 1);
    }
}

void subproc_sched(scheduler* scheduler) 
{
  while(1)
  {
    for(int i = 0;i < )
    subproc_schedule(scheduler);
  }
}

struct task* subproc(struct proc* p, size_t memsz, int flags) // subprocess
{
  struct task* t = (struct task*)kalloc(sizeof(struct task), KERN_MEM);
  t->size = memsz;
  t->perm = flags;
  t->stack = (char*)prallocheap(p, memsz);
  if(flags & IO_TASK)
      t->ofiles = p->ofiles;
  t->parent = p;
  return t;
}

void subproc_launch(struct scheduler* sched, struct task* task)
{
  subproc_schedule(sched);
}