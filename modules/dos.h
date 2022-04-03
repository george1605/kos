#pragma once 
#include "../lib.c"
#include "bga.h"
#include "screen.h"
#include "time.c"
#define DOS_VMEM 0xE00000 
#define VIRTMEM(x) (void*)(DOS_VMEM + x)
#define PAGEMEM(x) (void*)(0xFF0000 + x)

typedef struct {
   char ah;
   char al;
   char dh;
   char dl;

   int ax;
   int bx;
} cpureg;

struct WORDREGS {
    unsigned int  ax, bx, cx, dx, si, di, cflag, flags;
};

struct BYTEREGS {
    unsigned char al, ah, bl, bh, cl, ch, dl, dh;
};

union REGS {
    struct  WORDREGS x;
    struct  BYTEREGS h;
};

struct viewporttype {
    int left, top, right, bottom;
    int clip;
};

struct time {
    int second;
    int minute;
    int hour;
    int day;
    int month;
    int year;
};

void gettime(struct time* t) {
  if(t != NULL_PTR)
    filldate((struct rtcdate*)t);
}

// -- graphics --
enum grdrv { DETECT, CGA, MCGA, EGA, EGA64, EGAMONO, IBM8514, HERCMONO,
                      ATT400, VGA, PC3270 };
struct graph {
  void* fb; 
  int width;
  int height;
};

struct graph _initgraph(int* gd, int* gm){
 struct graph u = {0};
 if(gd == 0 || gm == 0)
   return u;

 if(*gd == DETECT){
   u.fb = getfb();
   u.width = 1920;
   u.height = 1080;
 }else if(*gd == VGA){
   u.fb = (void*)0xA0000;
   u.width = 320;
   u.height = 200;
 }else{
   //NULL
 }
 return u;
}

void _closegraph(struct graph t){
 t.width = 0;
 t.height = 0;
 t.fb = (void*)0;
}

void _putpixel(struct graph u, size_t x, size_t y, size_t color){
  *(u.fb + u.width * y + x) = color;
}

int _getpixel(struct graph t, size_t x, size_t y){
  int u = *(u.fb + u.width * y + x);
  return u;
}

#define evloop() while(1)
#define intr(arg) asm("int %0\n" : : "N"((arg)) : "cc", "memory") /* just an interrupt */

#define setebx(no) asm ("movl %1, %%ebx;" \
                     : "=r" ( no )  \
                     : "r" ( no )   \
                     : "%ebx"   \
                    )

#define getebx(no) asm ("movl %%ebx, %0;" \
                     : "=r" ( no )  \
                     : "r" ( no )   \
                     : "%ebx"   \
                    )

#define seteax(no) asm ("movl %1, %%eax;" \
                     : "=r" ( no )  \
                     : "r" ( no )   \
                     : "%eax"   \
                    )

#define geteax(no) asm ("movl %%eax, %0;" \
                     : "=r" ( no )  \
                     : "r" ( no )   \
                     : "%eax"   \
                    )

void readregs(struct REGS* in){
  geteax(in->x.ax);
  in->x.flags = readeflags();
}

void int86(int intno, union REGS* in, union REGS* out){
  seteax(in->x.ax);
  setebx(in->x.bx);
  intr(intno);
}

void int08h(){
  intr(0x08);
}

void int10h(){
  intr(0x10);
}

void int13h(){
  intr(0x13);
}

void int17h(){
  intr(0x17);
}

void int14h(){
  intr(0x14);
}

void int1Ah(){
  intr(0x1A);
}

void int15h() {
    intr(0x15);
}

int isphys(void* addr){ 
  if((int)addr > DOS_VMEM)
    return 0;
  return 1;
}

typedef struct _MPAGE {
   int size;
   void* phys;
   struct _MPAGE* next;
} mpage;

mpage* _getpg(){ /* gets the first page */
    mpage* i = PAGEMEM(0);
    i->phys = 0x0;
    return i;
}

void* _dmalloc(mpage* lmpage, int bytes){
 if(lmpage != 0){
  mpage* u = (lmpage + sizeof(mpage) + 1);
  u->phys = VIRTMEM(4);
  u->size = bytes;
  lmpage->next = u;
 }else{
  return NULL_PTR;
 }
}

void _clrpg(mpage* pg){
  while(*(pg->phys) != 0){
      *(pg->phys) = 0;
      pg->phys++;
   }
}

void* _dfree(mpage* pg){
  if(pg != 0){
    pg->size = 0;
    _clrpg(pg);
  }
}

void _bgasetup(int x, int y){
 if(x == 0 && y == 0){
   bgaset(470,540);
 }else{
   bgaset(x,y);
 }
}