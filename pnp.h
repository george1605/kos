#pragma once
#include "lib.c"
#include "drivers/blockdev.h"
#define JOYSTICK_PORT 0x200
#define JOYSTICK_RESET() outb(JOYSTICK_PORT,0)
#define JOYSTICK_READ() inb(JOYSTICK_PORT)

struct pnpdev {
  struct spinlock lock;
  size_t type;
  size_t port;
};

void pnplock(struct pnpdev u){
  acquire(&u.lock);
}

void pnpunlock(struct pnpdev u){
  release(&u.lock);
}

void pnpctl(struct pnpdev u, int req){
  if(u.lock.locked) return;
  
  switch(req){
    case 0:
     if(u.port == 0x200) JOYSTICK_RESET();
    break;
  }
}