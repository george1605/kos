#pragma once
#include "ioapic.h"
#include "smem.h"
#include "cpuem.h"
#include "vfs.h"
#define __PROC_UNSAFE static

#define X86ENV 1
#define ARMENV 2
#define AVRENV 4
#define X64ENV 8
#define MAINENV 0x4BB0000
#define SHUTDOWN 0x20
#define RESTART 0x21

int lpid = 1;
int glsig = 0;
typedef enum
{
  UNUSED,
  EMBRYO,
  STARTED,
  PAUSED,
  KILLED,
  ZOMBIE
} prstate;
typedef void (*func)(int argc, char **argv);

void cons(int argc, char **argv)
{
  if (argc == 0)
    return;
}

#define MAX_FDS 20

struct proc
{
  prstate state;
  struct proc *parent;
  struct vfile std[3]; // stdin, stdout, stderr
  int pid;
  int ret;
  char *stack;
  char *name;
  void *ctx; // used for fxrestor(), fxsave()
  int ssize;
  func f;
  struct file* ofiles;
} tproc, kproc;

void praddvfile(struct proc* p, struct vfile* vf)
{
 int fd = vf->fd;
 if(vf->fd > MAX_FDS) // remap the file descriptor to the file map
 {
    fd = fdremap(vf->fd, p->ofiles);
 }  
 p->ofiles[fd].flags = vf->status | F_VIRT;
 p->ofiles[fd].name = vf->name;
 p->ofiles[fd].parent = vf->parent;
}

void praddfile(struct proc* p, struct file* fi)
{
  if(p == NULL_PTR || fi == NULL_PTR) return;
  int fd = fi->fd;
  if(fi->fd > MAX_FDS)
    fd = fdremap(fi->fd, p->ofiles);
  p->ofiles[fd] = *fi;
}

struct prset
{
  struct proc* procs;
  size_t cnt, front, rear, size;
  struct spinlock lock;
} prlist;

void prappend(struct proc u)
{
  while(prlist.lock.locked)
    NOP();
  acquire(&prlist.lock);
  prlist.procs[++prlist.cnt] = u;
  release(&prlist.lock);
}

void sched()
{
  struct proc p = prlist.procs[prlist.front];
  prlist.front = (prlist.front + 1) % prlist.cnt;
  prlist.size--;
  prswap(p);
  delay(5000); // <-- this corresponds to the time frame for each process
  prswap(tproc);
  if(p.state != KILLED)
  {
    prlist.rear = (prlist.rear + 1) % prlist.cnt;
    prlist.procs[prlist.rear] = p;
    prlist.size++;
  }
}

struct proc prnew_k(char* name, int memSize)
{
  struct proc p;
  p.name = name;
  p.stack = (char*)vmap(NULL_PTR, memSize, 0, (vfile*)NULL_PTR);
  p.ssize = memSize;
  p.pid = ++lpid;
  p.parent = &tproc;
  sched();
  return p;
}


struct sleeplock
{
  struct spinlock lock;
  struct proc proc;
};

struct environ
{
  struct context *ctx;
  void *vars;
  int cpunum;
  int type;
};

char* strenv(char* name, char* val)
{
  char* buf = (char*)kalloc(128, KERN_MEM);
  size_t x = strlen(name);
  memcpy(buf, name, x);
  buf[x++] = '=';
  memcpy(buf + x, val, strlen(val));
  return buf;
}

struct sleeplock initsleep(struct proc u)
{
  struct sleeplock i;
  i.proc = u;
  acquire(&i.lock);
  return i;
}

void sleep(struct proc u)
{
  struct sleeplock f = initsleep(u);
  while (f.proc.state != KILLED && f.proc.stack != 0)
  { 
    xchg((size_t*)&f.proc.state, (size_t)KILLED);
  }
  release(&f.lock);
}

void* prsave(struct proc u) // saves the context and enters idle mode
{
  void* p = kalloc(512, KERN_MEM);
  fxsave(p);
  pridle(u);
  return p;
}

void prload(struct proc u, void* p) // loads the context from memory
{
  fxrestor(p);
  u.state = STARTED;
  prswap(u);
}

void pridle(struct proc u)
{
  u.state = PAUSED; // got it out the loop
  for(;;)
  {
    asm volatile("pause");
  }
}

void endsleep(struct sleeplock u)
{
  release(&u.lock);
  u.proc.state = STARTED;
}

struct cpu *mycpu(void)
{
  int id = lapicid();
  int i = 0;
  while (cpus[i].cid != id && i < 32)
  {
    i++;
  }
  return &(cpus[i]);
}

struct proc* myproc(void)
{
  struct cpu *c;
  struct proc *p;
  pushcli(c);
  c = mycpu();
  p = c->proc;
  popcli(c);
  return p;
}

// create a process with the same name, link with the parent proc + copy the mem
struct proc prfork()
{
  struct proc* parent = myproc();
  struct proc p = prnew_k(parent->name, parent->ssize);
  p.parent = parent;
  memcpy(p.stack, parent->stack, parent->ssize);
  return p;
}

// returns the process currently running
struct proc* prswitch(struct proc* p)
{
  struct cpu* cpu = mycpu();
  struct proc* lastp;
  if(cpu->proc->state != KILLED)
    fxsave(cpu->proc->ctx); // save it now
  lastp = cpu->proc;
  cpu->proc = p;
  return lastp;
}

void prcpy(struct proc a, struct proc b)
{ // copies the stack
  int n = 0;
  while (n < a.ssize && n < b.ssize)
  {
    *(b.stack++) = *(a.stack++);
    n++;
  }
}

void przom(struct proc a)
{
  a.state = ZOMBIE;
}

struct proc prdup(struct proc a)
{
  struct proc t;
  t.parent = a.parent;
  t.stack = a.stack;
  return t;
}

void prinit(struct proc u, struct proc *parent)
{
  u.pid = (++lpid);
  u.state = STARTED;
  u.stack = (char*)kalloc(64 * 1024, USER_MEM);
  if (parent == (struct proc *)0)
    u.parent = &tproc;
  else
    u.parent = parent;
  sched();
}

struct proc prcreat(char *prname)
{
  struct proc u;
  prinit(u, 0);
  u.name = prname;
}

struct proc proc_init()
{
  struct proc t;
  t.state = STARTED;
  t.pid = 1;
  t.stack = kalloc(100, KERN_MEM);
  t.f = cons;
  kproc = t;
  kproc.f(0, (char **)0);
}

void proc_exit()
{
  HALT();
  kproc.state = KILLED;
  kproc.parent = (struct proc *)0;
  free((int *)kproc.stack);
  kproc.ssize = 0;
  glsig = SHUTDOWN;
}

void prswap(struct proc* u)
{
  struct cpu* cpu = mycpu();
  fxsave(cpu->proc->ctx); // save the prev process
  cpu->proc = u;
  char** argv = (char**)kalloc(2 * sizeof(char*), USER_MEM);
  argv[0] = u->name;
  argv[1] = (char*)NULL_PTR;
  u->f(1, argv);
}

struct proc prnew(int pid)
{
  struct proc x;
  x.pid = pid;
  x.parent = &tproc;
  x.stack = alloc(0, 64);
  x.state = STARTED;
  return x; // <-- How could I forget this?
}

struct proc procid(int pid) // if it doesn't find a process, it creates one
{
  int i;
  for (i = 0; i < prlist.cnt; i++)
    if (prlist.procs[i].pid == pid)
      return prlist.procs[i];

  struct proc t = prnew(pid);
  prappend(t); // appends the new process to the list
  return t;
}

// call the function and change the stat of the process
void prcall(int pid, int argc, char** argv)
{
  struct proc p = procid(pid);
  if(p.state != EMBRYO) return;
  argv[0] = strdup(p.name); // puts the name into it even if argv[0] = NULL_PTR
  p.f(argc, argv);
  p.state = STARTED;
}

void waitpid(int pid)
{
  struct proc u = procid(pid);
  while(u.state == KILLED)
  {
    prswap(&tproc);
  }
}

void prsswap(struct proc prlist[], int procs)
{
  int a = 0;
  struct proc t = prlist[a];
  while (a < procs)
  {
    if (t.state != KILLED)
      prswap(&t);

    t = prlist[++a];
  }
}

int prkill(struct proc* u)
{
  if (u->pid < 1 || u->parent == 0)
    return;

  // HALT(); - well, not anymore, getting better at this things
  u->state = KILLED;
  u->pid = 0;
  free((int *)u->stack);
  u->stack = (char*)NULL_PTR;
  fxrestor(u->parent->ctx);
  prswap(u->parent);
  sched(); // reschedule 
  return u->ret;
}

#define MAX_EXITFUNCS 10
struct exithnd
{
  void(*f[MAX_EXITFUNCS])();
  int num_funcs;
};

void kprexit(struct proc* p, int code)
{
  struct exithnd* exit = (struct exithnd*)(p->stack + p->ssize - sizeof(struct exithnd));
  for(int i = 0;i < exit->num_funcs;i++)
      exit->f[i]();   
  p->ret = code;
  for(int i = 0;i < MAX_FDS;i++)
    if(p->ofiles[i].fd != -1)
      fclose(p->ofiles[i]); // close the files
  prkill(p); // kill the current proc
}

// log to the console when it exits
void klogproc(struct proc* p)
{
  printf("Process: pid=%i exited with code=%i", p->pid, p->ret);
}

void prend(struct proc u, int status)
{
  prkill(&u);
  kprint("Process ended with status: ");
}

void prexec(int argv, char **argc)
{
  if (tproc.state != KILLED)
    tproc.f(argv, argc);
}

void pushctx(struct context *ctx)
{
  if (ctx != 0)
  {
    asm volatile("movl %1, %%ebp"
                 : "=r"(ctx->ebp)
                 : "r"(ctx->ebp));
    asm volatile("movl %1, %%ebx"
                 : "=r"(ctx->ebx)
                 : "r"(ctx->ebx));
    asm volatile("movl %1, %%edi"
                 : "=r"(ctx->edi)
                 : "r"(ctx->edi));
    asm volatile("movl %1, %%esi"
                 : "=r"(ctx->esi)
                 : "r"(ctx->esi));
  }
}

void readctx(struct context *ctx)
{
  if (ctx != 0)
  {
    asm volatile("movl %%ebp, %1"
                 : "=r"(ctx->ebp)
                 : "r"(ctx->ebp));
    asm volatile("movl %%ebx, %1"
                 : "=r"(ctx->ebx)
                 : "r"(ctx->ebx));
    asm volatile("movl %%edi, %1"
                 : "=r"(ctx->edi)
                 : "r"(ctx->edi));
    asm volatile("movl %%esi, %1"
                 : "=r"(ctx->esi)
                 : "r"(ctx->esi));
  }
}

void fcall(uint64_t addr, struct context* x)
{
  pushctx(x);
  asm volatile("jmp %0" : : "r"(addr));
}

void pushregs()
{
  asm volatile("push %eax");
  asm volatile("push %ebx");
  asm volatile("push %ecx");
}

void popregs()
{
  asm volatile("pop %ecx");
  asm volatile("pop %ebx");
  asm volatile("pop %eax");
}

struct environ* envdef() // creates a default environment
{
  struct environ* x = TALLOC(struct environ);
  x->cpunum = mycpu()->cid;
  x->ctx = TALLOC(struct context);
  #ifdef __ARM__
    x->type = ARMENV;
  #elif defined(__AVR__)
    x->type = AVRENV;
  #else
    x->type = X86ENV;
  #endif
  readctx(x->ctx);
  return x;
}

void mmap(void* mem, int flags, struct proc p) // maps a memory region to a process
{
  if(mem == NULL_PTR) return;
  if(mem < KERN_MEM)
    p.stack = _vm(mem); // just temporary
}

// adds a heap to the process
void praddheap(struct proc* p)
{
  pagetable* tbl = (pagetable*)p->stack;
  add_to_ptable(tbl, (void*)0x1, p->stack + sizeof(pagetable)); 
}

void prdelheap(struct proc* p)
{
  if(p == NULL_PTR) p = myproc();
  pagetable* tbl = (pagetable*)p->stack;
  mem_unmap(tbl);
}

// looks for a unused cpu and starts the function there
void schedinit(struct proc* p)
{
  size_t cores, threads;
  int i;
  cpugetinfo(&cores, &threads);
  for(i = 0;i < cores;i++) {
    if(cpus[i].proc == NULL_PTR) {// no proc on cpu 
        cpus[i].proc = p;
        break;
    }
  }
  lapic_startup(i, (uint32_t)p->f);
  p->state = STARTED;
}

void envinit(struct environ *u)
{
  u->cpunum = mycpu()->cid;
  u->vars = kalloc(32, KERN_MEM);
  u->type = X86ENV;
  u->ctx = kalloc(sizeof(struct context), KERN_MEM);
  memcpy(u->vars, "PATH=/home\n", 12);
  readctx(u->ctx);
}

void envpush(struct environ *u)
{
  pushctx(u->ctx);
}

void envswap(struct environ *u, int cid)
{
  u->cpunum = cid;
}

void envfree(struct environ *u)
{
  free(u->ctx);
  free(u->vars);
  u->vars = NULL_PTR;
}

void envrun(struct environ *u, func f)
{
  if (u == NULL_PTR)
    return;

  f(1, (char **)u);
}

void envexec(struct environ* u, struct proc p)
{
  if(u == NULL_PTR)
    return;
  
  fxsave(NULL_PTR); // saves the regs
  pushctx(u->ctx);
  tproc = p;
  p.f(0, (char**)0);
}

void envrstor()
{
  fxrestor(NULL_PTR);
}

struct isolate
{
  char *stack;
  int files[8];
  int fcnt;
  int (*handler)(int);
};

struct isolate islcreat(int (*x)(int))
{
  struct isolate u;
  u.fcnt = 0;
  u.stack = kalloc(128, USER_MEM);
  memset(u.files, 0, 8);
  u.handler = x;
  return u;
}

int __isldef(int code)
{
  char *stk;
  if (code != 0)
  {
    stk = ((struct isolate *)code)->stack;
    *(stk + 1) = 0;
  }
  return 0;
}

struct isolate islcreatx(int (*x)(int), size_t ssize)
{
  struct isolate u;
  u.stack = kalloc(ssize, USER_MEM);
  memset(u.files, 0, 8);
  u.handler = x;
  return u;
}

int islrun(struct isolate i)
{
  if (i.stack == 0)
    i.stack = kalloc(128, USER_MEM);

  int p = i.handler((int)&i);
  free(i.stack);
  return p;
}

struct atomic
{
  struct spinlock alock; // lock for writing
  long value;
};