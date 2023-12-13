#pragma once
#include "lib.c"
#include "process.h"
#include "port.h"
#include "smem.h"
#include "time.c"
#include "fs.h"
#include "vfs.h"
#include "user.h"
#include "pci.h"
#define __SYSCALL static inline 
#define PORTMAX 0xFFFF
#define SYSRES 0xC0DEBA5E /* system reserved space */
int usermode = 0;
int sysmode = 1;

int *SMI_CMD;
char ACPI_ENABLE;
char ACPI_DISABLE;
int *PM1a_CNT;
int *PM1b_CNT;
short SLP_TYPa;
short SLP_TYPb;
short SLP_EN;
short SCI_EN;
char PM1_CNT_LEN;

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
  outw(PM1a_CNT, SLP_TYPa | SLP_EN);
}

__SYSCALL struct rtcdate * sys_time()
{
  struct rtcdate *u = (struct rtcdate *)0xDDFF00;
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

__SYSCALL int sys_open(void *arg1, void *arg2)
{
  struct vfile vf;
  struct file fil;

  vf = vfsopen(arg1);
  return vf.fd;
}

__SYSCALL void sys_mkdir(void *arg1, void *arg2)
{
  char *dname = (char *)arg1;
  struct file *parent = (struct file *)arg2;
  mkdir(dname, parent);
}

__SYSCALL void sys_vfsread(void* arg1, void* arg2)
{
  if(arg1 == NULL_PTR || arg2 == NULL_PTR)
    return;
  struct vfile* vf = (struct vfile*)arg1;
  int* sz = (int*)arg2;
  char* data = (char*)(arg2 + sizeof(int));
  vfsread(vf, data, *sz);
}

__SYSCALL void sys_write(void *arg1, void *arg2)
{
  struct buf *a = (struct buf *)arg2;
  a->flags = B_DIRTY;
  _write(sys_int(arg1), a, 512);
}

__SYSCALL void sys_read(void* arg1, void* arg2)
{
  struct buf *a = (struct buf *)arg2;
  a->flags = B_VALID;
  _read(sys_int(arg1), a, 512);
}

__SYSCALL void sys_rem(int fd)
{
  if(fd <= 3) // cannot remove stdin/stdout
    return;
  setfptr(fd, NULL_PTR); // just sets the filepointer to NULL
}

__SYSCALL void* sys_malloc(size_t size)
{
  return safe_alloc(size);
}

__SYSCALL void sys_free(void* ptr)
{
  struct malloc_block q;
  for(q = malloc_head;q != 0;q = q->next)
    if(q->mem == ptr)
      q->free = 0;
}

void sysc_handler(struct regs *r)
{
  switch (r->eax)
  {
  case 0x4:
    sys_open((void *)r->ebx, (void *)r->ecx);
    break;
  case 0x5:
    sys_write((void *)r->ebx, (void *)r->ecx);
    break;
  case 0xF:
    r->ebx = (size_t)sys_time();
    break;
  case 0x10:
    sys_sleep();
    break;
  case 0x11:
    r->ecx = sys_malloc(r->ebx);
    break;
  case 0x12:
    sys_free(r->ebx);
    break;
  case 0x13:
    sys_rem(r->eax);
  }
}

void sysc_load() // add the sysc_handler()
{
  idt_set_gate(0x80, (unsigned)sysc_handler, 0x08, 0x8F);
}

void* userm_malloc(size_t size)
{
  void* p;
  asm volatile("mov %eax, $0x11;"
               "mov %ebx, %0;"
               "int $0x80"
               :"r"(size));
  int ig;
  asm volatile("mov $0, %%eax":"=r"(p):"r"(ig));
  return p;
}

void userm_free(void* ptr)
{
  asm volatile("mov %eax, $0x12;"
               "mov %ebx, %0;"
               "int $0x80"
               :"r"(ptr));
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

struct rsdp_desc
{
  char sign[8];
  uint8_t checksum;
  char OEMID[6];
  uint8_t revision;
  uint32_t raddr;
} __attribute__((packed));

const char *rsdp_sign = "RSD PTR ";

struct rsdp_desc* rsdp_find()
{
  int a;
  for (a = 0x01; a < 0x7BFF; a += 16)
    if (strcmp(rsdp_sign, a) == 0)
      return (struct rsdp_desc *)a;

  for (a = 0xE0000; a < 0xFFFFF; a += 16)
    if(strcmp(rsdp_sign,a) == 0)
      return (struct rsdp_desc *)a;

  return (struct rsdp_desc*)NULL_PTR;
}

int acpi_enable(void)
{
  if ((inw((unsigned int)PM1a_CNT) & SCI_EN) == 0)
  {
    // check if acpi can be enabled
    if (SMI_CMD != 0 && ACPI_ENABLE != 0)
    {
      outb((unsigned int)SMI_CMD, ACPI_ENABLE); 
      int i;
      for (i = 0; i < 300; i++)
      {
        if ((inw((unsigned int)PM1a_CNT) & SCI_EN) == 1)
          break;
        wait(10);
      }
      if ((int)PM1b_CNT != 0)
        for (; i < 300; i++)
        {
          if ((inw((unsigned int)PM1b_CNT) & SCI_EN) == 1)
            break;
          sleep(10);
        }
      if (i < 300)
      {
        return 0;
      }
      else
      {
        return -1;
      }
    }
    else
    {
      return -1;
    }
  }
  else
  {
    return 0;
  }
}

void acpi_shutdown(void)
{
  if (SCI_EN == 0)
    return;

  acpi_enable();
  outw((unsigned int)PM1a_CNT, SLP_TYPa | SLP_EN);
  if (PM1b_CNT != 0)
    outw((unsigned int)PM1b_CNT, SLP_TYPb | SLP_EN);
}

void acpi_init(void)
{
  acpi_enable();
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
  prkill(tproc);
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

void _waitfunc(int argc, char **argv)
{
  struct proc i = GETPROC(argv);
  if (argc == 1)
  {
    while (1)
    {
      if (i.state == PAUSED)
        break;
    }
  }
}

struct rmentry
{
  void *ptr;
  int size;
};

struct _rmem
{
  struct rmentry entries[16];
  int n;
} rmtable;

void __SYSCALL sys_prwait(struct proc u)
{
  
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

void __SYSCALL sys_resmem(void *mem, size_t size)
{
  struct rmentry i;
  int *ptr = mem;
  if ((int)ptr > 1 && rmtable.n < 16)
  {
    *(ptr - 1) = SYSRES;
    *(ptr + size) = 0;
    i.ptr = ptr;
    i.size = size;
    rmtable.entries[++rmtable.n] = i;
  }
}

void restxt()
{ // reserves the text buffer at 0xB8000
  sys_resmem((void *)0xB8000, 80 * 25 + 1);
}

void resvid()
{ // reserves the video memory
  sys_resmem((void *)0xA0000, 200 * 320 + 1);
}
