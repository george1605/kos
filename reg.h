#pragma once
#define RINT 0x1
#define RDOUBLE 0x2
#define RLONG 0x4
#define RCHAR 0xF

#define GSTART 0x20
#define GFSON 0x21
#define GRAM2G 0x22
typedef union {
  int rint;
  double rdouble;
  long rlong;
  char rchar;
} regval;

struct _regen {
  regval value;
  int type;
  int info;
} regents[8];

void reg_init(){
  regval t = {1};
  regents[0].value = t;
  regents[0].type = RINT;
  regents[0].info = GFSON;
  return;
}
