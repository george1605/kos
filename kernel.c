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
  shell_as_start(); // start as the shell
  while(1){
    if(glsig == RESTART)
      acpi_reset();
      break;
  }
  kernel_close();
}
