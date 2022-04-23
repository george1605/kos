#pragma once
#include "process.h"
#include "elf.h"
#include "lib.c"
#include "port.h"
#include "mutex.h"
#define HDR_SECTION __attribute__((section(".hdr")))
#define KERNEL_FUNC 0x09
#define DRIVER_FUNC 0x0A
#define USER_FUNC 0x0B
#define LIB_FUNC 0x0C

#define INTVAR 4
#define BYTEVAR 1
#define WORDVAR 2
#define LLVAR 8

struct ElfToken symtable[10];
int symcnt = 0;

struct ElfToken symlook(uint64_t address)
{
  int a = 0;
  for(a = 0;a < 10;a++)
    if(symtable[a].addr == address)
      return symtable[a];
  
  struct ElfToken tkn = {.addr = NULL_PTR};
  return tkn;
}

void symadd(struct ElfToken x)
{
  if(symcnt == 10) return;
  symtable[symcnt++] = x;
}

func dlsym(char* name)
{
  for(int a = 0;a < 10;a++)
    if(strcmp(symtable[a].name, name) == 0)
      return (func)symtable[a].addr;
  
  return (func)NULL_PTR;
}

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

void bootload()
{
  struct ElfHeader* hdr = (struct ElfHeader*)0x10000;
  void(*entry)(void);
  readseg((char*)hdr, 4096, 0);
  if(!checkelf(hdr))
    perror("Could not load the OS.(corrupted)");
  entry = (void(*)(void))hdr->e_entry;
  entry();
}

void __ata_async(int argc, char** argv) // asynchronous IO
{
  if(argc < 3) return;
  if(argv[0][0] == 'r')
    ata_read((size_t)argv[1], (size_t)argc[2]);
  else
    ata_write_sector(&ata_primary_master, (size_t)argv[1], argv[2]);
}

void ata_asread(int ata, int sect)
{
  struct thread x = thcreat(tproc, 512);
  x.f = __ata_async;
  thrunp(x, cthread);
}