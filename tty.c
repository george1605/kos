#pragma once
#include "mem.h"
#include "lib.c"
#include "drivers/ioctl.h"
#include "vfs.h"
#define TTYBASE 0xBB0
#define MAXTTY 17
#define TTY_ERR 128
#define TTY_OPEN 1
#define TTY_WRITE 2
#define TTY_READ 4
#define TTY_LOCK 8
#define TTY_CLS 16

struct ttyport
{
  struct spinlock lock;
  int blkopen;
  size_t id;
  size_t flags;
  size_t iflags;
};

int ttyid = 0x200;

struct ttyport ttyalloc()
{
  struct ttyport u;
  u.id = ++ttyid;
  u.flags |= TTY_OPEN;
  initlock(&u.lock, NULL_PTR);
}

struct ttydev
{
  struct blkdev *device;
  int num;
  int flags;
  char *name;
  struct spinlock lock;
} ttys[MAXTTY];

int ttyinit(size_t num)
{
  if (num > MAXTTY)
    return -1;

  struct ttydev u;
  u.flags = TTY_OPEN;
  u.num = num;
  initlock(&u.lock, NULL_PTR);
  ttys[num] = u;
  return 0;
}

struct vfile ttymap(char *name, size_t num)
{
  struct vfile u;
  u.status = 0xF0;
  u.fd = TTYBASE + num;
  u.name = name;
  return u;
}

struct ttydev ttyopen(size_t num)
{
  if (ttys[num].flags == 0)
    ttyinit(num);
  return ttys[num];
}

void ttystart()
{
  if (!irq_isset(4)) // if serial is not connected
  {
    serial_install();
    serial_init();
  }
  ttyinit(0);
  systty[0] = ttymap("/home/tty/tty0.con", 0);
}

void ttyset(size_t num, struct blkdev *dev)
{
  ttys[num].device = dev;
}

void ttyclose(size_t num)
{
  ttys[num].flags |= TTY_CLS;
  release(&(ttys[num].lock));
}

char *ttyread(size_t num)
{
  if (num > MAXTTY)
    return NULL_PTR;
  if (ttys[num].lock.locked == 1)
    return NULL_PTR;

  return blkreads(ttys[num].device);
}

int ttywrite(size_t num, char *dev)
{
  if (num > MAXTTY)
    return -1;
  if (ttys[num].lock.locked == 1)
    return -1;

  blkwrites(ttys[num].device, dev);
  return 0;
}

void ttyraise(size_t num)
{ // closes the TTY and adds the TTY_ERR flag
  ttyclose(num);
  ttys[num].flags |= TTY_ERR;
}

void ttyctl(int num, int func)
{
  switch (func)
  {
  case 0:
    ttyinit(num);
    break;
  case 2:
    ttyclose(num);
    break;
  case 7:
    ttyraise(num);
    break;
  }
}