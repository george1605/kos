#include "elf.h"
#include "stdlib.c"

static int decode_hex(const char* x) // no '0x'
{
  int p = 0;
  while(*x)
  {
      if(*x >= 'a')
        p = p * 16 + (*x - 'a' + 10);
      else
        p = p * 16 + (*x - '0');
    x++;
  }
  return p;
}

int main(int argc, char** argv)
{
    void* mem;
    struct ElfHeader* n = TALLOC(struct ElfHeader);
    if(argc <= 2)
        return 1;

    if(!strcmp(argv[1], "-m"))
    {
        mem = (void*)decode_hex(argv[2]);
        memcpy(n, mem, sizeof(struct ELfHeader));
        printf("e_entry: %i", n->e_entry);
    }
    return 0;
}