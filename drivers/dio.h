#pragma once
#include "../port.h"
#include "../mem.h"
#define DIO_VENDOR_ID  44
#define DIO_MODULE_ID  120

#define DIO_BASE       0x600000       
#define DIO_END        0x1000000       
#define DIO_DEVSIZE    0x10000         

#define DIOII_BASE     0x01000000      
#define DIOII_END      0x20000000     
#define DIOII_DEVSIZE  0x00400000    

struct diores {
  char* name;
  long start;
  long end;
};

struct diodev {
  struct diores res[3];
  struct spinlock lock;
};

void diowrite(size_t port, size_t portlvl){

}

void diolock(struct diodev u){
  acquire(&u.lock);
}

void diounlock(struct diodev u){
  acquire(&u.lock);
}