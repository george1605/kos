#pragma once
#include "lib.c"
#include "port.h"
#define PERIPH_BASE 0x40000000U
#define PPBI_BASE 0xE0000000U
#define GPIO_BASE 0x20200000
#define GPIO_TIMBASE 0x20003000
#define GPIO_AUXBASE 0x20215004
#define GPIO_SEL0 GPIO_BASE
#define GPIO_SEL1 (GPIO_BASE + 0x4)
#define GPIO_SEL2 (GPIO_BASE + 0x8)
#define GPIO_SEL3 (GPIO_BASE + 0xC)
#define MMIO_R(x) (*(volatile unsigned int*)x)
#define MMIO_W(x,y) (*(volatile unsigned int*)x = y)

struct pin {
    int num;
    int active;
    long mmio;
};

static uint8_t* pin_status;

struct vport { // virtual port
    int num;
    char stack[8];
    int size;
};

void voutb(struct vport u, char value){
    u.stack[++u.size] = value;
}

char vinb(struct vport u){
    return u.stack[u.size];
}

struct vport vpmap(int port){
    struct vport u;
    u.num = port;
    u.size = 0;
    memset(u.stack,0,8);
    return u;
}

void pin_write(struct pin u,unsigned int x){
    if(u.mmio == 0) u.mmio = GPIO_SEL0;
    MMIO_W(u.mmio,x);
}

unsigned int pin_read(struct pin u){
    if (u.mmio == 0) u.mmio = GPIO_SEL0;
    return MMIO_R(u.mmio);
}

static struct pin pin_init(int n){
  struct pin u;
  u.active = 1;
  u.mmio = GPIO_BASE + 0x4 * n;
  u.num = n;
  return u;
}

void pin_request(int n)
{
  if(pin_status[n] == 1)
   iowait(0);
  else
   pin_status[n] = 1;
}
