#pragma once
// Let's use the syscalls
#include "usermode.h"

#define F_ERROR 0xFF
#define NULL (void*)0 // NULL_PTR only defined in kern mode
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

struct __pthread {
	int tid;
	void* (*entry)(void*);
	void* arg;
};

time_t time(time_t* time){
  if(time == NULL)
    return userm_time();
  else {
    *time = userm_time();
    return *time;
  } 
}

void* malloc(size_t bytes){
  return userm_malloc(bytes);
}

char to_upper(char u){
  if(u >= 65 && u <= 91)
    return (char)(u + 32);
}

void* calloc(size_t x, size_t y)
{
  void* p = malloc(x * y);
  memset(p, 0, x * y);
  return p;
}

void mkdir(char* dir)
{
  userm_mkdir(dir);
}

int fork()
{
  return userm_fork();
}

void exit(int code)
{
  userm_exit(code); // from here we are no more
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

void _setbuf(FILE* fp, char* buf)
{
  if(fp->_buf != NULL) return;
  fp->_buf = buf;
}

void _fwrite(void* data, size_t size, size_t count, FILE* fp)
{
  size_t sz = size * count;
  if(fp->_buf != NULL)
  {
    memcpy(fp->_buf, data, min(sz, fp->_bufsize)); // buffering
  }
}

int _ferror(FILE* fp)
{
  return fp->_flags & F_ERROR;
}

void _fprintf(FILE* fp, char* fmt, ...)
{
  va_list list;
  char buf[300];
  va_start(fmt, list);
  char* fmt = va_arg(list, char*);
  vsnprintf(buf, sizeof buf, fmt, list);
  _fwrite(buf, sizeof buf, 1, fp);
  va_end(list);
}

int _remove(char* name)
{
  int fd = sys_open(name, "r");
  sys_rem(fd); // remove the file
  sys_fclose((void*)fd);
}

void _fclose(FILE *x)
{
  if (!x)
    return;

  sys_fclose((void*)x->_fd);
  x->_open = (char)0;
}

void _fread(void* buf, int size, int cnt, FILE* ptr)
{
  size_t bytes = size * cnt;
  if(ptr->_buf != NULL && ptr->_flags != B_NONE)
  {
    memcpy(buf, ptr->_buf, min(bytes, ptr->_bufsize)); // copy data from buffer
    ptr->_flags = B_NONE;
    return;
  }
  
}

FILE *freopen(const char *filename, const char *mode, FILE *stream)
{
  FILE* n = (FILE*)malloc(sizeof(FILE));
  
}

int fileno(FILE* f)
{
  if(f == NULL)
    return -1;
  
  return f->_fd;
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
