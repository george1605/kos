#pragma once
#include "../stdlib.h"
#include "../mem.h"
#include "../elf.h"
#define TYPE_TOKEN 1 // like 'int'
#define OPER_TOKEN 2
#define NUMB_TOKEN 3
#define STRN_TOKEN 4
#define PREP_TOKEN 6

struct token {
  char* text;
  int type;
};

typedef struct token token_t;

void* __builtin_malloc(int _Bytes)
{
  return malloc(bytes);
}

void __builtin_free(void* _Block)
{
  free((int*)_Block);
}

void __builtin_trap()
{
  for(;;);
}

void __builtin_macros(char* fname)
{
  #define __FILE fname
  #define __GNU__ 1
}

void gccexec(int argc, char** argv)
{
  if(argc <= 0) return;
  
  char* fin, fout;
  if(strcmp(argv[1], "-o") == 0 || strcmp(argv[1], "-c") == 0)
    fout = argv[2];
  fin = argv[3];
  fcreat(fout);
  __builtin_macros(fin);
}