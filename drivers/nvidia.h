#include "../mem.h"
#include "../port.h"
#define VIDEO_IRQ 9

void* nvid_addrs[5] = {
  (void *)0xfa000000,
  (void *)0xe0000000,
  (void *)0xf6000000,
  (void *)0xfd000000
};

struct gbuf {
  void* addr;
  size_t off;
  size_t size;
};

struct gpos {
  int x;
  int y;
};

struct gscreen { //multiscreening
  int width;
  int height;
  int bpp;
} cscr;

void nvsetres(int width,int height,int bpp){
  cscr.width = width;
  cscr.height = height;
  if(bpp == 0)
    cscr.bpp = 32;
  else 
    cscr.bpp = bpp;
}

void nvgetscr(int scrno){
  
}

struct gbuf* nvallocb(){
  struct gbuf* u = (struct buf*)kalloc(sizeof(struct gbuf),KERN_MEM);
  u->addr = nvid_addrs[0];
  return u;
}

struct gbuf* nvsetbuf(struct gbuf* u,size_t off){
  u->addr = nvid_addrs[0];
  u->off = off;
  u->size = 4096;
  return u;
}

void nvclrbuf(struct gbuf* o){
  free(o->addr);
  o->off = 0;
  o->size = 0;
}

void nvputbuf(struct gbuf* buf,struct gpos pos,int color){
  int loc = pos.y * cscr.width + pos.x;
  if(buf != 0 && buf->addr != 0){
    buf->addr[++loc] = (color & 0xffffff00) >> 3;
    buf->addr[++loc] = (color & 0xffff00) >> 2;
    buf->addr[++loc] = (color & 0xff00) >> 1;
  }
}
