#include "fs.h"
#define ISO_RES_START 0x0
#define ISO_RES_END 0x7

struct ISOHeader
{
  char typecode;
  char ident[5]; // 'CD001'
  char version;
  char unused; // 0x0
};
