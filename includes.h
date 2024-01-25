#pragma once
#include "lib.c"
#include "fpu.h"
#include "idt.c"
#include "gdt.c"
#include "isr.c"
#include "irq.c"
#include "port.h"
#include "disk.h"
#include "mem.h"
#include "tty.c"
#include "mp.h"
#include "desktop.h"
#include "kb.c"
#include "mouse.h"
#include "speaker.h"
#include "process.h"
#include "uart.c"
#include "user.h"
#include "pit.c"
#include "system.h"
#include "pnp.h"
#include "drivers/soundcard.h"

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

void switch_long_mode() {
    uint64_t cr4;
    asm volatile("mov %%cr4, %0" : "=r"(cr4));
    cr4 |= (1 << 5);  // Set the 5th bit for Long Mode
    asm volatile("mov %0, %%cr4" :: "r"(cr4));

    uint64_t efer;
    asm volatile("rdmsr" : "=A"(efer) : "c"(0xC0000080));
    efer |= (1 << 8);
    asm volatile("wrmsr" :: "c"(0xC0000080), "A"(efer));

    asm volatile(
        "mov $1f, %0 \n\t"  
        "ljmp $0x8, $0x0 \n\t" 
        "1:"
        :
        : "m"(0x1000)
    );
}

void init()
{
  idt_init();
  gdt_init();
  isrs_init();
  irq_init();
  fpu_init();
  sse_init();
  ioapic_init(); // init these, would need for multiproc.
  lapicinit();
  mpinit(); // SMP
  proc_init();
  pci_init();
  ata_init();
  kbd_init();
  mouse_init();
  timer_init();
  dsp_init();
  switch_long_mode();
  syscinit();
  kprint("x64 Mode Activated. (Respect +)");
  fs_init();
  vfsinit();
  user_init();
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
  int i;
  for(i = 0;i < prlist.cnt;i++)
    kprexit(&prlist.procs[i], 0);
  pci_unload();
  irq_uninstall_handler(0); 
  irq_uninstall_handler(1);
  vfsrem();
  sfree(); // deallocate kern memory
  acpi_shutdown(); // shutdown
}
