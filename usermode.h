#pragma once
#include "types.h"
#define SYSCALL_EXIT 1
#define SYSCALL_READ 2
#define SYSCALL_WRITE 3
#define SYSCALL_MALLOC 4
#define SYSCALL_FREE 5
#define SYSCALL_OPEN 6
#define SYSCALL_MKDIR 7

void* userm_malloc(size_t size)
{
  uint32_t p;
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