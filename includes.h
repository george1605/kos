#pragma once
#include "lib.c"
#include "fpu.h"
#include "reg.h"
#include "idt.c"
#include "gdt.c"
#include "isr.c"
#include "irq.c"
#include "port.h"
#include "disk.h"
#include "mem.h"
#include "tty.c"
#include "desktop.h"
#include "kb.c"
#include "mouse.h"
#include "speaker.h"
#include "process.h"
#include "uart.c"
#include "pit.c"
#include "system.h"
#include "pnp.h"

#define FATAL_MEMCOR 0x8
#define FAULTY_DEV 0x9
#define BAD_FS 0x7
typedef volatile char *charp;
typedef volatile unsigned int *uintp;
typedef volatile int *intp;

void getsys()
{
  uintp k = (volatile unsigned int *)(0x25);
}

void sendferr(int code)
{ // sends a fatal error ( line 23 )
  glsig = code | 0x10;
}

void islfadd(struct isolate u, int fd)
{
  u.fcnt++;
  u.files[u.fcnt] = fd;
}

struct vfile islmap(struct isolate isl)
{
  struct vfile u;
  u.mem = (void *)(&isl);
  u.status = 0x4;
  u.fd = 0;
  return u;
}

void envwrite(struct environ *v) // writes the environ variables
{
  if (v == NULL_PTR)
    return;

  ata_write_sector(&ata_primary_master, 20, v->vars);
}

void init()
{
  idt_init();
  gdt_init();
  isrs_init();
  sysc_load();
  irq_init();
  fpu_init();
  proc_init();
  pci_init();
  kbd_init();
  mouse_init();
  timer_init();
  fs_init();
  vfsinit();
  ttyinit(0);
#ifdef __PNP__
  JOYSTICK_RESET();
#endif
}

void kernel_setup(int a)
{
  timer_active = a;
  clr();
  init();
  STI();
}

void gui_setup()
{
  vidsetup();
  DrawDesktop();
}

void mysh(char *comm)
{
  if (comm == 0)
    return;
  if (strcmp("mkpanel", comm))
  {
    vbecolor = 0x00ff4f;
  }
}

void kernel_close()
{
  pci_unload();
  irq_uninstall_handler(0); // removes the IRQ for Keyboard and
  irq_uninstall_handler(1);
  vfsrem();
}
