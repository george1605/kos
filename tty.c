#pragma once
#include "mem.h"
#include "lib.c"
#include "vfs.h"
#include "fs.h"
#define TTYBASE 0xBB00
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
  void(*getc)(char*);
  void(*putc)(char);
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

int is_tty(int fd)
{
  if(fd < TTYBASE || fd > TTYBASE + MAXTTY)
    return 0;
  return (ttys[fd - TTYBASE].getc != NULL_PTR); // if registered function
}

struct ttydev ttyopen_k(size_t num)
{
  if (ttys[num].flags == 0)
    ttyinit(num);
  return ttys[num];
}

static int tty_getid(char* name)
{ 
    
    return 0;
}

struct ttydev ttyopen(char* name, int flags)
{
  struct ttydev dev;
  if(flags == F_NEW)
  {
      dev.name = name;
      return dev;
  } else {
      for(int i = 0;i < MAXTTY;i++)
        if(strcmp(name, ttys[i].name))
          return ttys[i];
  }
}

int ttyread(size_t num, char* data)
{
  if (num > MAXTTY)
    return 0;
  if (ttys[num].lock.locked == 1)
    return 0;

  while(*data != '\0')
    ttys[num].getc(data++);
}

int ttywrite(size_t num, char *dev)
{
  if (num > MAXTTY)
    return -1;
  if (ttys[num].lock.locked == 1)
    return -1;

  while(*dev)
    ttys[num].putc(*dev++);
  return 0;
}

void tty_read_ext(char* name, char* buffer, ext2_gen_device* dev, void* priv)
{
    if(memncmp(name, "/tty-", 5) != 0)
      return;
    name += 4;
    uint32_t i = 0, j = 0;
    while(name[i] != '\0')
    {
      j = j * 10 + (name[i] - '0');
      i++;
    }
    ttyread(j, buffer);
}

void tty_write_ext(char* name, char* buffer, size_t len, ext2_gen_device* dev, void* priv)
{
    if(memncmp(name, "/tty-", 5) != 0)
      return;
    name += 4;
    uint32_t i = 0, j = 0;
    while(name[i] != '\0')
    {
      j = j * 10 + (name[i] - '0');
      i++;
    }
    ttywrite(j, buffer);
}

uint8_t tty_exist_ext(char* name, ext2_gen_device* dev, void* priv)
{
  if(memncmp(name, "/tty-", 5) != 0)
      return;
  name += 4;
  uint32_t i = 0, j = 0;
  while(name[i] != '\0')
  {
    j = j * 10 + (name[i] - '0');
    i++;
  }
  if(j > MAXTTY)
    return 0;
  return 1;
}

void ttystart()
{
  if (!irq_isset(4)) // if serial is not connected
  {
    serial_install();
    serial_init();
  }
  ttys[0].putc = uartputc;
  ttys[0].getc = uartgetc;
  filesystem* fs = kalloc(sizeof(filesystem), KERN_MEM);
  fs->name = "TTY";
  fs->read = tty_read_ext;
  fs->writefile = tty_write_ext;
  fs->exist = tty_exist_ext;
  mount("/home/dev/tty", fs);
}

void ttyswap(int x, int y)
{
  if(x >= MAXTTY || y >= MAXTTY) return;
  struct ttydev n;
  n = ttys[y]; // swaps
  ttys[x] = n;
  ttys[y] = ttys[x]; 
}

void ttyclose(size_t num)
{
  ttys[num].flags |= TTY_CLS;
  release(&(ttys[num].lock));
}

void ttyraise(size_t num)
{ // closes the TTY and adds the TTY_ERR flag
  ttyclose(num);
  ttys[num].flags |= TTY_ERR;
}

void ttywait(size_t num)
{
    ttys[num].flags |= TTY_LOCK;
    acquire(&ttys[num].lock);
    DNOP();
}

void ttyplay(size_t num)
{
    ttys[num].flags &= ~TTY_LOCK;
    release(&ttys[num].lock);
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