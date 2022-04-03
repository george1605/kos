#pragma once
#include "../lib.c"
#include "../mem.h"
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

void* __builtin_malloc(int _Bytes){
  return alloc(0,bytes);
}

void __builtin_free(void* _Block){
  free((int*)_Block);
}