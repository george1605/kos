#pragma once
#include "../lib.c"
#include "../ide.c"
#include "../mem.h"

struct ZipEntry {
  int sign;
  uint16_t version;
  uint16_t bitflag;
  uint16_t method;
  uint16_t last_tm;
  uint16_t last_dt;
  size_t crc32;
  size_t cmp_sz;
  size_t uncmp_sz;
};

struct ZipSection {
  struct ZipEntry* entries;

};

struct ZipEntry* readzip(struct buf* b){
  struct ZipEntry* u = kalloc(SIZESTR(ZipSection),USER_MEM);
  if(b != 0 && u != 0){
   u->sign = bcasti(b,0);
   u->version = bcastw(b,4);
   u->bitflag = bcastw(b,6);
   u->method = bcastw(b,8);
   u->last_tm = bcastw(b,10);
   u->last_dt = bcastw(b,12);
   u->crc32 = bcasti(b,14);
   u->cmp_sz = bcasti(b,16);
   u->uncmp_sz = bcasti(b,20);
  }
  return u;
}