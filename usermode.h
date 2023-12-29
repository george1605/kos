#pragma once
#include "types.h"
#define SYSCALL_EXIT 1
#define SYSCALL_READ 2
#define SYSCALL_WRITE 3
#define SYSCALL_MALLOC 4
#define SYSCALL_FREE 5
#define SYSCALL_OPEN 6
#define SYSCALL_MKDIR 7

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
  asm volatile("int $0x80" : "=a"(p) : "a"(SYSCALL_MALLOC));
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