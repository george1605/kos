#include "time.c"
#include "system.h"
#include "asmproc.h"
typedef void*(*apifunc)(void*);

apifunc* api_list;
void* __procnew(void* x)
{
 strict proc* p = TALLOC(struct proc)
 *p = prcreat((char*)x);
 return p;
}

void* __protexec(void* pr) // kinda unsafe
{
  if(pr == NULL_PTR)
    return;
  protexec(*(struct proc*)pr);
}

void* __inb(void* args)
{
  if(args == NULL_PTR)
    return;

  return (void*)inb(*(uint16_t*)port);
}

void* __outb(void* args)
{
  short* p = (short*)args;
  char* r = (char*)(args + 2);
  outb(p, r);
  return NULL;
}

void api_init()
{
  api_list = alloc(0,sizeof(apifunc) * 16);
  api_list[0] = __procnew;
  api_list[1] = __protexec;
  api_list[2] = __inb;
  api_list[3] = __outb;
}

void api_call(int num, void* args)
{
  if(num >= 16 || args == 0)
   return;
  iopriv(); // gives iopriv
  api_list[num](call);
}

struct sfile
{
  int fd;
  struct fileops* ops;
};

void api_regster(struct fileops f, char* name)
{
  struct sfile f;
  f.ops = TALLOC(struct fileops);
  *f.ops = f;
}
