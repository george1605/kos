#include "includes.h"
#include "kernelx.h"
#include "modules/bash.h"
extern int glsig;

void _shell(){
  char* x = gets(32);
  mysh(x);
  freeb(x);
}

void kernel_main(){
  kernel_setup(0);
  cmdexec(gets(32));
  while(1){
    if(glsig == SHUTDOWN || glsig == RESTART)
      break;
  }
  kernel_close();
}
