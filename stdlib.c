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

typedef struct __File
{
  char* _buf;
  size_t _bufsize;
  size_t _flags;
  int _fd;
  char* _perm;
  char _open;
} FILE;

FILE stdfiles[3];
#define stdin &stdfiles[0]
#define stdout &stdfiles[1]
#define stderr &stdfiles[2]

time_t time(int k){
  if(k < 0)
    return 0;
  return cmos_read(SECS);
}

void* malloc(size_t bytes){
  if(bytes > 0x100000)
    return _vm(0xFF00);
  return userm_malloc(bytes);
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

static int _fflags(char* fmod)
{
  if(strcmp(fmod, "w"))
    return F_WRITE;
  if(strcmp(fmod, "r"))
    return F_READ;
  return F_NEW; // the default flag is F_NEW
}

static char* _fperm(char* mod)
{
  if(strcmp(mod, "r"))
    return "--r";
  if(strcmp(mod, "w+"))
    return "-rw";
  return "";
}

FILE *_fopen(char *name, char *mod)
{
  FILE* x = (FILE*)malloc(sizeof(FILE));
  x->_open = 1;
  x->_perm = _fperm(mod);
  x->_fd = sys_open(name, mod);
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

void _fread(void* buf, int size, int cnt, FILE* ptr)
{
  size_t bytes = size * cnt;
  if(ptr->_buf != NULL && ptr->_flags != B_NONE)
  {
    memcpy(buf, ptr->_buf, min(bytes, ptr->_bufsize)); // copy data from buffer
    ptr->_flags = B_NONE;
    return 0;
  }
  struct vfile vf;
  vf.fd = ptr->_fd;
  vf.mem = NULL;
  vfsread(vf, buf, bytes);
}

void _fclose(FILE* x)
{
  x->_open = 0;
  free(x);
}

FILE *freopen(const char *filename, const char *mode, FILE *stream)
{
  FILE* n = (FILE*)malloc(sizeof(FILE));
  
}

int fileno(FILE* f)
{
  if(f == NULL_PTR)
    return -1;
  
  return f->_fd;
}

void printf(const char* format, void* args)
{
  int i;
  for(i = 0;format[i] != 0;i++)
  {
    if(format[i] != '%' && format[i+1] != 'i')
      putch(format[i]);
    else
      puts(atoi(args[0]));
  }
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
