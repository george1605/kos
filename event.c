#pragma once
#include "fs.h"
#include "lib.c"
#define KILL_PROC 0x16
#define KILL_THREAD 0x20
#define CREAT_FILE 0x24
#define DELET_FILE 0x28

#define CPU_OBJ 0x0
#define MEM_OBJ 0x1
#define FILE_OBJ 0x2 //general file
#define DA_OBJ 0x4
#define IMG_OBJ 0x8
#define DEV_OBJ 0xA //device
#define WEB_OBJ 0xF
struct event {
  void* object;
  size_t type;
  char* msg;
};

struct kobj {
  int type;
  void* mem;/// allocated memory
  void(*handler)(struct event _Event);
};

void sendev(struct event u){
  if(u.object){
    struct kobj* o = u.object;
    o->handler(u);
  }
}

void delobj(struct kobj u){
  free((int*)u.mem);
  u.type = 0;
}

void readcpu(struct kobj u){
  u.type = CPU_OBJ;
}

struct kobj readobj(struct buf* b){
  struct kobj i;
  i.type = b->data[0];
}