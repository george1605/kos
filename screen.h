#pragma once
#include "lib.c"
#include "dma.h"
#include "vesa.h"
#include "vfs.h"
#include "mem.h"
#define VGA_AC_INDEX      0x3C0
#define VGA_AC_WRITE      0x3C0
#define VGA_AC_READ       0x3C1
#define VGA_MISC_WRITE    0x3C2
#define VGA_SEQ_INDEX     0x3C4
#define VGA_SEQ_DATA      0x3C5
#define VGA_DAC_READ_INDEX  0x3C7
#define VGA_DAC_WRITE_INDEX 0x3C8
#define VGA_DAC_DATA      0x3C9
#define VGA_MISC_READ     0x3CC
#define VGA_GC_INDEX      0x3CE
#define VGA_GC_DATA       0x3CF
#define VGA_CRTC_INDEX    0x3D4
#define VGA_CRTC_DATA     0x3D5
#define VGA_INSTAT_READ   0x3DA
#define VGA_NUM_SEQ_REGS  5
#define VGA_NUM_CRTC_REGS 25
#define VGA_NUM_GC_REGS   9
#define VGA_NUM_AC_REGS   21
#define VGA_NUM_REGS      (1+VGA_NUM_SEQ_REGS+VGA_NUM_CRTC_REGS+VGA_NUM_GC_REGS+VGA_NUM_AC_REGS)
#define VBUFFER_EX 0xE0000000
#define PREFERRED_VY 4096
#define PREFERRED_B 32

#define COLOR(_r, _g, _b)((uint8_t)( \
    (((_r) & 0x7) << 5) |       \
    (((_g) & 0x7) << 2) |       \
    (((_b) & 0x3) << 0)))

#define COLOR_R(_index) (((_index) >> 5) & 0x7)
#define COLOR_G(_index) (((_index) >> 2) & 0x7)
#define COLOR_B(_index) (((_index) >> 0) & 0x3)
#define PALETTE_MASK 0x3C6
#define PALETTE_READ 0x3C7
#define PALETTE_WRITE 0x3C8
#define PALETTE_DATA 0x3C9

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

void fbsetres(uint16_t x, uint16_t y)
{
   outports(0x1CE, 0x04);
   outports(0x1CF, 0x00);
   outports(0x1CE, 0x01);
   outports(0x1CF, x);
   outports(0x1CE, 0x02);
   outports(0x1CF, y);
   outports(0x1CE, 0x03);
   outports(0x1CF, PREFERRED_B);
   outports(0x1CE, 0x07);
   outports(0x1CF, PREFERRED_VY);
   outports(0x1CE, 0x04);
   outports(0x1CF, 0x41);
   outports(0x1CE, 0x01);
   x = inports(0x1CF);
   fb_resolution_x = x;
   fb_resolution_s = x * 4;
   fb_resolution_y = y;
   fb_resolution_b = 32;
   #if defined(SCREEN_SIZE)
    #undef SCREEN_SIZE
   #endif
   #define SCREEN_SIZE (fb_resolution_x * fb_resolution_y)
}

void fbsetback(){ // set-ups the back buffer
   if(fb_resolution_x && fb_resolution_y) fbsetres(320,200);
   BBUFFER = kalloc(fb_resolution_x * fb_resolution_y,KERN_MEM);
}

unsigned char mode_320_200_256[]={
   0x63,
   0x03,
   0x01,
   0x0F,
   0x00,
   0x0E,
   /* CRTC */
   0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F,
   0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x9C, 0x0E, 0x8F, 0x28,   0x40, 0x96, 0xB9, 0xA3,
   0xFF,
   /* GC */
   0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F,
   0xFF,
   /* AC */
   0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
   0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
   0x41, 0x00, 0x0F, 0x00,   0x00
};

void screen_init() {

    outportb(PALETTE_MASK, 0xFF);
    outportb(PALETTE_WRITE, 0);
    unsigned char i;
    for (i = 0; i < 255; i++) {
        outportb(PALETTE_DATA, (((i >> 5) & 0x7) * (256 / 8)) / 4);
        outportb(PALETTE_DATA, (((i >> 2) & 0x7) * (256 / 8)) / 4);
        outportb(PALETTE_DATA, (((i >> 0) & 0x3) * (256 / 4)) / 4);
    }

    outportb(PALETTE_DATA, 0x3F);
    outportb(PALETTE_DATA, 0x3F);
    outportb(PALETTE_DATA, 0x3F);
}

void write_registers(unsigned char *regs){
   unsigned i;
   outportb(VGA_MISC_WRITE, *regs);
   regs++;
   /* write SEQUENCER regs */
   for(i = 0; i < VGA_NUM_SEQ_REGS; i++)
   {
      outportb(VGA_SEQ_INDEX, i);
      outportb(VGA_SEQ_DATA, *regs);
      regs++;
   }

   outportb(VGA_CRTC_INDEX, 0x03);
   outportb(VGA_CRTC_DATA, inportb(VGA_CRTC_DATA) | 0x80);
   outportb(VGA_CRTC_INDEX, 0x11);
   outportb(VGA_CRTC_DATA, inportb(VGA_CRTC_DATA) & ~0x80);

   regs[0x03] |= 0x80;
   regs[0x11] &= ~0x80;
   for(i = 0; i < VGA_NUM_CRTC_REGS; i++)
   {
      outportb(VGA_CRTC_INDEX, i);
      outportb(VGA_CRTC_DATA, *regs);
      regs++;
   }

   for(i = 0; i < VGA_NUM_GC_REGS; i++)
   {
      outportb(VGA_GC_INDEX, i);
      outportb(VGA_GC_DATA, *regs);
      regs++;
   }

   for(i = 0; i < VGA_NUM_AC_REGS; i++)
   {
      (void)inportb(VGA_INSTAT_READ);
      outportb(VGA_AC_INDEX, i);
      outportb(VGA_AC_WRITE, *regs);
      regs++;
   }

   (void)inportb(VGA_INSTAT_READ);
   outportb(VGA_AC_INDEX, 0x20);
}

void screen_clear(unsigned char color) {
    memset(VBUFFER, color, SCREEN_SIZE);
}

int RGB(int r,int g,int b){
   return (r | (g << 8) | (b << 16));
}

void setpixel(char* BUF,int x,int y,int color){
 if(BUF != 0)
   *(BUF + y * fb_resolution_x + x) = color;
}

void screen_new(){
 write_registers(mode_320_200_256);
 screen_init();
 screen_clear(COLOR(7,7,7));
}

#define GET_ALPHA(color) ((color >> 24) & 0x000000FF)
#define GET_RED(color)   ((color >> 16) & 0x000000FF)
#define GET_GREEN(color) ((color >> 8)  & 0x000000FF)
#define GET_BLUE(color)  ((color >> 0)  & 0X000000FF)

#define VIDEO_TYPE_NONE 0x00
#define VIDEO_TYPE_COLOUR 0x20
#define VIDEO_TYPE_MONOCHROME 0x30

void fbmount(){
  sysvf[15] = vfsmap("/home/dev/vid0.dev",(void*)getfb()); // creates a virtual control file 
}

void fbclear(size_t color){
   int i;
   size_t* ptr = (size_t*)VBUFFER_EX;
   for(i = 0; i < 1920 * 1080; i++){
      *(ptr + i) = color; 
   }
}
 
uint16_t vidgetinfo(void){
  const uint16_t* bda_detected_hardware_ptr = (uint16_t*)0x410;
  return (*bda_detected_hardware_ptr);
}
 
int viddetect(void){
  return (vidgetinfo() & 0x30);
}

struct vscreen
{
   int s_pid; 
   int s_fd; // allocated file descriptor
   uint16_t s_width, s_height;
   void* s_mem;
};

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
