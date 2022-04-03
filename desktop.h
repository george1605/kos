#pragma once
#include "fs.h"
#include "gui.c"

struct desktop {
  struct file* files;
  uint8_t* vidbuf;
  struct taskbar toolbar;
  icon background;
};

void DrawDesktop(){
  struct rect i;
  i.height = 40;
  i.width = 1920;
  i.x = 0;
  i.y = 0;
  DrawRectX(i,0xF);
}