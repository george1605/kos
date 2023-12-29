#pragma once
#define FL_IF 0x00000200 
#define ERR_INTRON 0x3A
#define ERR_UNKNOWNAPIC 0x3B
#define CPU_ID 0x1
#define CPU_STRUCT 0x2 // get cpu structure
#define CPU_PROC 0x3
#define CPU_SETUP 0x4
#include "lib.c"
#include "ioapic.h"

#define cpuid(level, a, b, c, d)                        \
  asm ("cpuid\n\t"                                        \
           : "=a" (a), "=b" (b), "=c" (c), "=d" (d)        \
           : "0" (level))

#define cpuid_count(level, count, a, b, c, d)                \
  asm ("cpuid\n\t"                                        \
           : "=a" (a), "=b" (b), "=c" (c), "=d" (d)        \
           : "0" (level), "2" (count))

#define is_x16() sizeof(void*) == 2
#define is_x64() sizeof(void*) == 4
#define is_x86() sizeof(void*) == 8

extern struct proc* myproc(void);
void cpuctl(int id, int req, int *params)
{
  switch (req)
  {
    case CPU_ID:
     if(params == 0) return;
     cpuid(params[0], params[1], params[2], params[3], params[4]);
    break;
    case CPU_STRUCT:
     *(struct cpu**)params = mycpu();
    break;
    case CPU_PROC:
     *(struct proc**)params = myproc();
    case CPU_SETUP:
     uint32_t addr = *(uint32_t*)params;
     lapic_startup(id, addr);
    default:
     raise(-1);
    break;
  }
}

void* getrbp()
{
  volatile int* val;
  asm volatile("mov %%rbp, %0"
               : "=r"(val));
  return (void*)val;
}

void* fxsave(void* ptr)
{
  if(ptr == NULL_PTR) ptr = (void*)0x1001000;
  asm volatile("fxsave64 (%0)" ::"r"((int*)ptr): "memory");
  return ptr;
}

void fxrestor(void* ptr)
{
  if (ptr == NULL_PTR) ptr = (void *)0x1001000;
  asm volatile("fxrstor64 (%0)" ::"r"((int*)ptr) : "memory");
}

void cpugetinfo(size_t* no_cores, size_t* no_threads)
{
  unsigned int eax, ebx, ecx, edx;
  cpuid(1, eax, ebx, ecx, edx);
  *no_cores = (ebx >> 16) & 0xFF;
  *no_threads = ecx & 0xFFFF;
}

void kprintinfo()
{
  size_t cores, threads;
  cpugetinfo(&cores, &threads);
  kprint("Number of cores: ");
  kprintint(cores);
  kprint(", number of threads:");
  kprintint(threads);
}