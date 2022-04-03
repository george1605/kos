#pragma once
#include "../gui.c"
typedef struct window win_t;
typedef struct kApp {
  struct proc process;
  win_t mainwin; 
  win_t* windows;
  int wincnt;
} app_t;

win_t kInitWin(){
  struct window u;
  u.active = 1;
  DrawWindow(u);
  return u;
}

void kSetBuffer(win_t window, void* bytes){
 if(bytes != 0)
  window.front = bytes;
}

void kFreeWin(win_t u){
  DestroyWindow(u);
}

void kDrawPixel(win_t w, int x, int y, int color){
  struct pos u;
  u.x = w.area.x;
  u.y = w.area.y;
  
}

app_t kInitApp(char* name){
  app_t u;
  u.process = prcreat(name);
  u.windows = (win_t*)alloc(0,32);
  u.mainwin = kInitWin();
  u.wincnt = 0;
  return u;
}

void kFreeApp(app_t tapp){
  int i;
  kFreeWin(tapp.mainwin);
  for(i = 0;i < tapp.wincnt;i++)
   kFreeWin(tapp.windows[i]);

  free((int*)tapp.windows);
  tapp.wincnt = 0;
  prkill(tapp.process);
}