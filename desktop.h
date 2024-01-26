#pragma once
#include "fs.h"
#include "gui.c"
#include "mouse.h"
#define MAX_WINDOWS 16

struct {
  struct window* windows[MAX_WINDOWS];
  void(*arange)();
} desktop;

void desktop_show(int wid)
{
  desktop.windows[wid].active = 1;
  wmout.show(desktop.windows[wid]);
  desktop.arange();
}

int desktop_alloc(struct window* win)
{
  for(int i = 1;i < MAX_WINDOWS;i++) {
    if(desktop.windows[i] == NULL_PTR) {
      desktop.windows[i] = win;
      return i;
    }
  }
}

int desktop_is_active(int wid)
{
  return desktop.windows[wid]->active == 1;
}

void desktop_set_main(struct window* win)
{
  desktop.windows[0] = win;
}

void desktop_trigger()
{
  kprexit(myproc(), 0);
}

void desktop_remove(int wid)
{
  int i;
  if(wid == 0) // if it is the main one, it clears all the windows
  {
    for(i = 1;i < MAX_WINDOWS;i++)
      if(desktop.windows[i] != NULL_PTR)
        wmout.remove(desktop.windows[i]);
    desktop_trigger();
  }
  wmout.remove(desktop.windows[i]); // it does free(), it must
  desktop.windows[i] = NULL_PTR; // mark as empty
  desktop.arange();
}

// give the desktop control to user
void desktop_trigger_mouse(int x, int y)
{
  if(x > 10 && y < 100)
    desktop_trigger();
}

void desktop_at_exit()
{
  desktop_remove(0); // remove all of them
}

void desktop_main(int argc, char** argv)
{
  if(!wmout.scrn)
  {
    kos_wm(&wmout);
  }
  struct window* win = wmout.create("desktop_win", 0, 0, wmout.scrn->s_width, wmout.scrn->s_height);
  wmout.show(win);
  desktop_set_main(win); // set as the main window and all the others are just children
  while(1) {
    if(glsig == 0x20){
        break;
    }
  }
  kprexit(myproc(), 0); // exit here
}

void desktop_as_start()
{
  struct proc* p = prnew_k("desktop", 64 * 1024);
  p->f = desktop_main;
  addexit(p, desktop_at_exit);
  schedinit(p);
}