#pragma once
#define FL_IF 0x00000200 
#define ERR_INTRON 0x3A
#define ERR_UNKNOWNAPIC 0x3B
#include "lib.c"
//emulates the CPU ;)
struct _cpu {
  char regs[4];
  int pc;
} cpu;

void movax(char val){
  cpu.regs[0] = val;
}

void movbx(char val){
  cpu.regs[1] = val;
}

void cpuinc(){
  cpu.pc++;
}

void cpudec(){
 if(cpu.pc > 0)
   cpu.pc--;
}

void cpuclr(){
 cpu.regs = {0,0,0,0};
 cpu.pc = 0;
}

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

void cpuctl(int id, int req, int *params)
{
  switch (req)
  {
    case 0:
     if(params == 0) return;
     cpuid(params[0], params[1], params[2], params[3], params[4]);
    break;
    case 1:
     
    break;
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