#pragma once
#include "lib.c"
#include "process.h"
#include "elf.h"
#include "port.h"
#include "time.c"
#include "fs.h"
#include "vfs.h"
#include "user.h"
#include "pci.h"
#include "acpi.h"
#include "gui.c"
#define __SYSCALL static inline 
#define SYSCALL_EXIT 1
#define SYSCALL_READ 2
#define SYSCALL_WRITE 3
#define SYSCALL_MALLOC 4
#define SYSCALL_FREE 5
#define SYSCALL_OPEN 6
#define SYSCALL_MKDIR 7
#define SYSCALL_FORK 8
#define PORTMAX 0xFFFF
#define SYSRES 0xC0DEBA5E /* system reserved space */
int usermode = 0;
int sysmode = 1;

#if _POSIX >= 2
#define POSIX_ARG_MAX 4096
#define POSIX_LINK_MAX 8
#define POSIX_MAX_CANON 255
#define POSIX_MAX_INPUT 255
#define POSIX_NAME_MAX 14
#define POSIX_PIPE_BUF 512
#define POSIX_SSIZE_MAX 32767
#define POSIX_STREAM_MAX 8
#define SEG_ALIGN __attribute__((aligned(POSIX_PIPE_BUF)))
#endif
#if _POSIX >= 3
#define POSIX_PATH_MAX 4096
#define POSIX_USERNAME_MAX 32
#define POSIX_SSIZE_MAX 32767
#define POSIX_PIPE_BUF 512
#define POSIX_PIPEX_BUF 4096
#endif

void stacktrace()
{
  // TO DO!
}

struct eptr
{
  void *ebp;
  void *esp;
};

__SYSCALL int sys_int(void *arg1)
{
  if (arg1 != 0)
    return *(int *)arg1;
  return -1;
}

__SYSCALL void sys_sleep()
{
  acpi_shutdown();
}

__SYSCALL struct rtcdate * sys_time()
{
  struct rtcdate *u = (struct rtcdate *)prallocheap(myproc(), sizeof(struct rtcdate));
  filldate(u);
  return u;
}

__SYSCALL void sys_exec(void *arg1, void *arg2)
{
  if (arg1 != 0)
  {
    prexec(*(int *)arg1, (char **)arg2);
  }
  else
  {
    prexec(0, (char **)arg2);
  }
}

__SYSCALL void sys_fabort()
{
  CLI();
  stacktrace(); // prints a stack trace
  while(1)
    HALT();
}

__SYSCALL int sys_open(void *arg1, void *arg2)
{
  int fd = vfscheck((char*)arg1); // gets the fd of a virtual file
  if(fd > -1)
    return fd;

  int fd = _open((char*)arg1, (int)arg2);
  struct proc* pr = myproc();
  pr->ofiles[fdremap(fd, pr->ofiles)].flags &= (int)arg2;
  return fd;
}

__SYSCALL void sys_mkdir(void *arg1, void *arg2)
{
  char *dname = (char *)arg1;
  mkdir(dname, (int)arg2);
}

__SYSCALL void sys_vfsread(void* arg1, void* arg2)
{
  if(arg1 == NULL_PTR || arg2 == NULL_PTR)
    return;
  struct vfile* vf = (struct vfile*)arg1;
  int* sz = (int*)arg2;
  char* data = (char*)(arg2 + sizeof(int));
  vfsread(*vf, data, *sz);
}

__SYSCALL void sys_fclose(void* arg1)
{
  struct proc* p = myproc();
  p->ofiles[(int)arg1].open = 0; // not open!
}

__SYSCALL void sys_write(void *arg1, void *arg2)
{
  _write((int)arg1, (struct buf*)arg2, 0);
}

__SYSCALL void sys_read(void* arg1, void* arg2)
{
  fsread((char*)arg1, (char*)arg2);
}

__SYSCALL int sys_fork()
{
  struct proc* p = prfork();
  return p->pid;
}

__SYSCALL void sys_rem(char* name)
{
  if(!strcmp(name, "/home"))
    return;
  ext2_removefile(name, fs_dev, (ext2_priv_data*)fs_dev->priv);
}

__SYSCALL void* sys_malloc(size_t size)
{
  return prallocheap(myproc(), size);
}

__SYSCALL void sys_free(void* ptr)
{
  prfreeheap(myproc(), ptr); // ARENA ALLOCATOR!
}

__SYSCALL void sys_setuid(int uid)
{
  setuid(uid);
}

__SYSCALL void sys_userls(int gid, char* list, size_t size)
{
  struct group group;
  struct buf* buffer = (struct buf*)kalloc(sizeof(struct buf), KERN_MEM);
  int fd = prfopen(myproc(), "/home/users", F_READ);
  group_load(buffer, gid, &group);
  user_ls(&group, list, size);
}

__SYSCALL void sys_ioctl(int fd, size_t cmd, size_t args)
{
  ioctl(fd, cmd, args);
}

#define MAX_SYSCALLS 20
void* syscalls[MAX_SYSCALLS];

void sysc_add(int id, void* func)
{
  if(id > MAX_SYSCALLS)
  {
    kprint("Cannot add syscall (id > MAX_SYSCALLS)");
  }
  syscalls[id] = func;
}

void sysc_handler(struct regs *r)
{
  void* handler = syscalls[r->eax];
  int ret;
  asm volatile("push %1 \n"
               "push %2 \n"
               "push %3 \n"
               "push %4 \n"
               "push %5 \n"
               "call *%6 \n"
               "pop %%ebx \n"
               "pop %%ebx \n"
               "pop %%ebx \n"
               "pop %%ebx \n"
               "pop %%ebx \n"
               : "=a"(ret)
               : "r"(r->edi), "r"(r->esi), "r"(r->edx), "r"(r->ecx),
                 "r"(r->ebx), "r"(handler));
  r->eax = ret;
}

void switch_userm()
{
  asm volatile("  \
     cli; \
     mov $0x23, %ax; \
     mov %ax, %ds; \
     mov %ax, %es; \
     mov %ax, %fs; \
     mov %ax, %gs; \
                   \
     mov %esp, %eax; \
     pushl $0x23; \
     pushl %eax; \
     pushf; \
     pushl $0x1B; \
     push $1f; \
     iret; \
   1: \
     ");
  usermode = 1;
  sysmode = 0;
}

void switch_kernm()
{
  iopriv();
  usermode = 0;
  sysmode = 1;
}

void asm_syscall(uint64_t a)
{
  asm volatile("syscall");
}

struct pollfd
{
  int fd;
  short events;
  short revents;
};

void sys_poll()
{
}

struct pres
{
  union
  {
    int fd;
    long devfd;
    void *buffer;
  } value;
  int type;
};

struct file getfile(char *name)
{
  struct file u;
  return u;
}

void __SYSCALL sys_abort()
{
  prkill(myproc());
}

uint32_t __SYSCALL sys_memsz()
{ 
  uint32_t total;
  uint16_t lowmem, highmem;

  outportb(0x70, 0x30);
  lowmem = inportb(0x71);
  outportb(0x70, 0x31);
  highmem = inportb(0x71);

  total = lowmem | highmem << 8;
  return total;
}

void setcore(uint64_t base)
{
#ifdef _X86_
  asm volatile("wrmsr"
               :
               : "c"(0xc0000101), "d"((uint32_t)(base >> 32)), "a"((uint32_t)(base & 0xFFFFFFFF)));
  asm volatile("wrmsr"
               :
               : "c"(0xc0000102), "d"((uint32_t)(base >> 32)), "a"((uint32_t)(base & 0xFFFFFFFF)));
#endif
}

struct proc __SYSCALL sys_execv(char *path, char *argv[])
{
  int argc = 0;
  while (argv[argc] != 0)
  {
    argc++;
  }
  struct proc u = prcreat(path);
  u.f(argc, (char **)argv);
  return u;
}

#define GETPROC(x) *(struct proc *)(x[0]) // gets an proc structure from char**

struct rmentry
{
  void *ptr;
  int size;
};

void __SYSCALL sys_prwait(int pid)
{
  int state;
  waitpid(pid, &state); // just as that!
}

void __SYSCALL sys_fcall(uint64_t addr, void* _regs) 
{
  struct context* u = (struct context*)_regs;
  if(u == NULL_PTR)
    return;
  if(addr == (uint64_t)_write) // cannot execute a function that is not a syscall
    fcall((uint64_t)sys_write, u);
  else if(addr == (uint64_t)_read) // prevents corrupting the FS
    fcall((uint64_t)sys_read, u);
  else
    fcall(addr, u);
}

__SYSCALL void sys_mprotect(void* ptr, size_t size, int prot)
{
  struct proc* p = myproc();
  pagetable* tbl = (pagetable*)p->stack;
  tlb_flush((uint64_t)tbl);
}

__SYSCALL void sys_atexit(void(*f)())
{
  addexit(myproc(), f); // that should be it...
}

__SYSCALL void sys_setdisplay(int id, int* last)
{
  if(last != NULL_PTR)
    *last = wmout.scrn->s_fd;
  wmout.scrn = scrn_req(id);
}

__SYSCALL struct window* sys_createwin(char* name, int x, int y, int w, int h)
{
  return wmout.create(name, x, y, w, h);
}

void sys_exit(void* arg1)
{
  struct proc* p = myproc();
  kprexit(p, (int)arg1);
}

__SYSCALL void sys_insmod(char* file)
{
  if(!fs_dev->fs->exist(file, fs_dev, fs_dev->priv))
    return;
  mod_load_file(file);
}

__SYSCALL void sys_delmod(char* file)
{
  mod_unload(file);
}

// add more if needed
void syscinit()
{
  sysc_add(1, sys_exit);
  sysc_add(2, sys_read);
  sysc_add(3, sys_write);
  sysc_add(4, sys_malloc);
  sysc_add(5, sys_free);
  sysc_add(6, sys_open);
  sysc_add(7, sys_mkdir);
  sysc_add(8, sys_execv);
  sysc_add(9, sys_fork);
  sysc_add(10, sys_ioctl);
  sysc_add(18, sys_insmod);
  sysc_add(19, sys_delmod);
}