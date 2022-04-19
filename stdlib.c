#pragma once
#include "lib.c"
#include "fs.h"

#define NULL NULL_PTR
#define TRUE 1
#define FALSE 0
#define bitor |
#define or ||
#define bitand &
#define and &&

typedef uint32_t time_t;
typedef uint64_t xtime_t; // extended UNIX Time
typedef struct file FILE;

time_t time(int k){
  if(k == 0)
    return 0;
}

void* malloc(int bytes){
  if(bytes > 0x100000)
    return _vm(0xFF00);
  return alloc(0,bytes);
}

char to_upper(char u){
  if(u >= 65 && u <= 91)
    return (char)(u + 32);
}

char to_lower(char u){
  if(u >= 97 && u <= 123)
    return (char)(u - 32);
}

char* strlow(char* u){
  int a;
  for(a = 0;u[a] != 0;a++)
    if (u[a] >= 65 && u[a] <= 91)
      u[a] += 32;
}

FILE* _fopen(char* name, char* mod)
{
  return NULL;
}

void _fclose(FILE* x)
{
  if(x)
    fclose(*x);
}

#define STACK(type)  struct stack_ ## type { \
                      type* buffer;         \
                      int size;  \
                      int pos;         \
                     }

#define QUEUE(type)  struct queue_ ## type { \
                      type* buffer;         \
                      int size;  \
                      int pos;         \
                     }
