#pragma once
#include "process.h"
#include "elf.h"
#include "lib.c"
#include "port.h"
#define HDR_SECTION __attribute__((section(".hdr")))
#define KERNEL_FUNC 0x09
#define DRIVER_FUNC 0x0A
#define USER_FUNC 0x0B
#define LIB_FUNC 0x0C

#define INTVAR 4
#define BYTEVAR 1
#define WORDVAR 2
#define LLVAR 8

struct stackfr {
  void* sstart;
  int nvar;
  int nfunc;
  int ssize;
  int sseek;
  struct spinlock lock;
};

struct port {
  int isnetw; // is network port or just an i/o port
  int num;
  struct spinlock lock;
};

struct kentry {
  func function;
  int flag;
};

void* creatvar(struct stackfr _frame, int bytes){ // returns 0 if the frame is locked
  void* u = 0; 
  if(!_frame.lock.locked){
    u = alloc(_frame.sstart, bytes);
  }
  _frame.nvar++;
  return u;
}

void freevar(struct stackfr _frame, void* loc){
  if(!_frame.lock.locked && loc > 1){
    freeb((char*)(loc));
  }
  _frame.nvar--;
}

void lockfr(struct stackfr _frame){
  acquire(&_frame.lock);
}

void unlockfr(struct stackfr _frame){
  release(&_frame.lock);
}

struct stackfr creatfr(){
  struct stackfr i;
  initlock(&i.lock,"stackframe");
  i.nfunc = 0;
  i.nvar = 0;
  return i;
}

void deletfr(struct stackfr fr){
  release(&fr.lock);
  free(fr.sstart);
  fr.nvar = fr.nfunc = 0;
  fr.sseek = fr.ssize = 0;
}

void setmflag(void* mem, int flag){
  int u = *(int*)mem;
  if(mem > 1)
   *(int*)mem = u | flag;
} 

int chkmflag(void* mem, int flag){
  if(mem > 1)
   return (*(int*)mem & flag);
  return 0;
}

int readport(struct port u){
  if(!u.lock.locked){
    if(!u.isnetw)
      return (int)inportb(u.num);
  }
}

void lockport(struct port u){
  acquire(&u.lock);
}

void unlockport(struct port u){
  release(&u.lock);
}