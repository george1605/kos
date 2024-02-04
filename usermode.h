#pragma once
#include "types.h"
#define SYSCALL_EXIT 1
#define SYSCALL_READ 2
#define SYSCALL_WRITE 3
#define SYSCALL_MALLOC 4
#define SYSCALL_FREE 5
#define SYSCALL_OPEN 6
#define SYSCALL_MKDIR 7
#define SYSCALL_EXECV 8
#define SYSCALL_FORK 9
#define SYSCALL_IOCTL 10
#define SYSCALL_INSMOD 18
#define SYSCALL_DELMOD 19

#define F_NEW 1
#define F_READ 2
#define F_WRITE 4
#define F_RDWR 8
#define F_DIR 32
#define F_DEV 64 /* device file */
#define F_EXEC 0xFF
#define F_VIRT 0x100
#define B_NONE 0x20

void* userm_malloc(size_t size)
{
  uint64_t p;
  asm volatile("int $0x80" : "=a"(p) : "O"(SYSCALL_MALLOC), "D"(size));
  return (void*)p;
}

void userm_free(void* ptr)
{
  asm volatile("int $0x80" ::"a"(SYSCALL_FREE), "b"(ptr));
}

void userm_exit(int code)
{
  asm volatile("int $0x80" ::"a"(SYSCALL_EXIT), "b"(code));
}

void userm_mkdir(char* dir)
{
  asm volatile("int $0x80" ::"a"(SYSCALL_MKDIR), "b"(dir)); 
}

int userm_open(char* file, int perms) 
{
  int result;
  asm volatile(
      "movl %1, %%ebx\n\t"   
      "movl %2, %%ecx\n\t"   // file pointer
      "movl $%3, %%eax\n\t"  // syscall number
      "int $0x80\n\t"
      "movl %%eax, %0"        // store the result in 'result'
      : "=r" (result)         // output operand
      : "r" (file), "r" (perms), "i" (SYSCALL_OPEN)  // input operands
      : "%ebx", "%ecx", "%eax"  // clobbered registers
  );
  return result;
}

int userm_fork()
{
  int pid;
  asm volatile("int $0x80" : "=a"(pid) : "a"(SYSCALL_FORK));
  return pid;
}

void userm_ioctl(int fd, size_t cmd, size_t arg) {
    asm volatile(
        "movq %[syscall_nr], %%rax\n\t"
        "movq %[fd], %%rdi\n\t"
        "movq %[cmd], %%rsi\n\t"
        "movq %[arg], %%rdx\n\t"
        "int $0x80"
        :
        : [syscall_nr] "i" (SYSCALL_IOCTL), [fd] "r" (fd), [cmd] "r" (cmd), [arg] "r" (arg)
        : "rax", "rdi", "rsi", "rdx", "rcx", "memory"
    );
}

void userm_insmod(char* file)
{
  asm volatile("int $0x80" ::"a"(SYSCALL_INSMOD), "b"(file));
}

void userm_delmod(char* file)
{
  asm volatile("int $0x80" ::"a"(SYSCALL_DELMOD), "b"(file));
}