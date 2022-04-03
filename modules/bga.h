#pragma once
#include "../port.h"
#include "../lib.c"
#define BGA_ADDRESS 0x01CE
#define BGA_DATA 0x01CF // next port 
#define BGA_REG_XRES 0x1
#define BGA_REG_YRES 0x2
#define BGA_REG_BPP 0x3
#define BGA_REG_ENABLE 0x4

struct bgares {
  int width;
  int height;
  int bpp;
};

int bgaread(size_t address){
  out16(BGA_ADDRESS, address);
  return in16(BGA_DATA);
}

void bgawrite(size_t address, size_t data){
  out16(BGA_ADDRESS, address);
  out16(BGA_DATA, data);
}

void bgaset(int width,int height){
  bgawrite(BGA_REG_XRES,width);
  bgawrite(BGA_REG_YRES,height);
  bgawrite(BGA_REG_BPP,32);
}