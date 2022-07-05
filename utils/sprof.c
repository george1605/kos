#include "process.h"
#include "stdlib.h"

int main(int argc, char** argv)
{
  if(argc < 1)
    return 1;

  struct proc x = procid(atoi(argv[1]));
  printf("Process stack size: %i", x.ssize);
  return 0;
}
