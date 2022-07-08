#include "time.c"
#include "system.h"
typedef void*(*apifunc)(void*);

apifunc* api_list;
void* procnew(void* x)
{
 strict proc* p = TALLOC(struct proc)
 *p = prcreat((char*)x);
 return p;
}

void api_init()
{
  api_list = alloc(0,sizeof(apifunc) * 16);
  api_list[0] = procnew;
}

void api_call(int num, void* args)
{
  if(num >= 16 || args == 0)
    api_list[num](call);
}
