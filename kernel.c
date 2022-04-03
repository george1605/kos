#include "includes.h"
#include "kernelx.h"
extern int glsig;

void _shell(){
  char* x = gets(32);
  mysh(x);
  freeb(x);
}

void kernel_main(){
  kernel_setup(0);
  gui_setup();
  while(1){
    if(glsig == SHUTDOWN || glsig == RESTART)
      break;
  }
  kernel_close();
}
