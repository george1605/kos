#pragma once
// Let's use the syscalls
#include "system.h"
#include "fs.h"
#include "time.c"

#define NULL NULL_PTR
#define RAND_MAX ((1 << 31) - 1)
#define RAND_MIN -RAND_MAX - 1
#define TRUE 1
#define FALSE 0
#define bitor |
#define or ||
#define bitand &
#define and &&

typedef uint32_t time_t;
typedef uint64_t xtime_t; // extended UNIX Time
typedef struct file FILE;

FILE stdfiles[3];
#define stdin &stdfiles[0]
#define stdout &stdfiles[1]
#derine stderr &stdfiles[2]

time_t time(int k){
  if(k < 0)
    return 0;
  return cmos_read(SECS);
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

void* calloc(int x, int y)
{
  return alloc(0, x * y); 
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
  return u;
}

int _fflags(char* fmod)
{
  if(strcmp(fmod, "w"))
    return F_WRITE;
  if(strcmp(fmod, "r"))
    return F_READ;
  return F_NEW; // the default flag is F_NEW
}

FILE *_fopen(char *name, char *mod)
{
  FILE* x = (FILE*)malloc(sizeof(FILE));
  x->open = 1;
  x->flags = _fflags(mod);
  x->name = name;
  return x;
}

void _fclose(FILE *x)
{
  if (x)
  {
    idewait(0);
    free(x);
  }
}

void _fread(void* buf, int size, int cnt, FILE* PTR)
{
   
}

void _fclose(FILE* x)
{
  if(x)
    fclose(*x);
}

int fileno(FILE* f)
{
  if(f == NULL_PTR)
    return -1;
  
  return f->fd;
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
