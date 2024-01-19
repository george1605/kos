#pragma once
#include "lib.c"
#include "dma.h"
#include "vesa.h"
#include "vfs.h"
#include "mem.h"
#include "./drivers/mesa/setup.c"
#define NSCREENS 8 // If you hav more than that, goodbye!

static char *VBUFFER = (char*)0xA0000;
static char *BBUFFER;
uint16_t fb_resolution_x = 0;
uint16_t fb_resolution_y = 0;
uint16_t fb_resolution_b = 0;
uint32_t fb_resolution_s = 0;

size_t* getfb(){
  size_t* u = (size_t*)*(size_t*)0x9028;
  return u;
}

int RGB(int r,int g,int b){
   return (r | (g << 8) | (b << 16));
}

#define VIDEO_TYPE_NONE 0x00
#define VIDEO_TYPE_COLOUR 0x20
#define VIDEO_TYPE_MONOCHROME 0x30

uint16_t vidgetinfo(void){
  const uint16_t* bda_detected_hardware_ptr = (uint16_t*)0x410;
  return (*bda_detected_hardware_ptr);
}

#define VID_VGA 0x1
#define VID_VESA 0x2
#define VID_INTLGPU 0x3
#define VID_NVDIA 0x4 // <-- "we" don't like this :)
#define VID_REQ 0x0

// returns the vfile from list if already setup, else sets it up
// according to the flags
struct vfile* vidvirt(int flags)
{
   struct vfile vf;

   if(sysvf[6].mem != NULL_PTR || flags == 0)
      return &sysvf[6];

   switch(flags)
   {
      case VID_VGA:
         vf.mem = (void*)0xA0000;
         vf.size = 64 * 1024; // 64 KB of VGA memory
      break;
      case VID_VESA:
         vf.mem = (void*)getfb();
         vf.size = 1920 * 1080 * 4;
      break;
      #ifdef _USE_MESA_
      case VID_MESA:
         // TO DO!
      break;
      #endif
      default:
         vf.mem = fb_info.mem;
         vf.size = fb_info.res_x * fb_info.res_y * fb_info.res_b; 
   }
   sysvf[6] = vf;
   return &sysvf[6];
}

// requests the video buffer by the process
// maps to the virtual space
// links it with the stdout (std[1])
void vidreq()
{
   struct proc* p = myproc();
   struct vfile* vf = vidvirt(0);
   vf->mem = vmap(vf->mem, vf->size, 0, NULL_PTR);
   vfslink2(vf, &(p->std[1]));
}
 
int viddetect(void){
  return (vidgetinfo() & 0x30);
}

struct vscreen
{
   int s_pid; 
   int s_fd; // allocated file descriptor
   uint16_t s_width, s_height, s_bpp;
   void* s_mem;
   struct spinlock s_lock;
} scrn_list[NSCREENS];

struct vscreen* scrn_req(int id)
{
   struct vscreen* s = &scrn_list[id];
   struct vfile* vf;
   if(id == 0)
   {
      vf = vidvirt(0);
      s->s_mem = vf->mem;
   }
   return (struct vscreen*)NULL_PTR;
}

void scrn_clear(struct vscreen* s, unsigned long color)
{
   acquire(&(s->s_lock));
   int i;
   for(i = 0;i < s->s_width * s->s_height;i++)
      memcpy(s->s_mem, (uint8_t*)&color, s->s_bpp);
   release(&(s->s_lock));
}

// for monochrome
void scrn_wbit(struct vscreen* scrn, size_t p)
{
    char* mem = (char*)scrn->s_mem;
    mem[p / 8] |= (1 << (p % 8));
}

void scrn_ubit(struct vscreen* scrn, size_t p)
{
    char* mem = (char*)scrn->s_mem;
    mem[p / 8] &= ~(1 << (p % 8));
}

char scrn_rbit(struct vscreen* scrn, size_t p)
{
     char* mem = (char*)scrn->s_mem;
     return mem[p / 8] & (1 << (p % 8));
}

void scrn_free(struct vscreen* scrn)
{
    scrn->s_mem = NULL_PTR;   
}