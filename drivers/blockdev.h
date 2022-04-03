#pragma once
#include "../fs.h"
#include "../port.h"
#include "../lib.c"
#include "../mem.h"
#include "../uart.c"
#define MAINPORT 0
#define STATUSPORT 1

#define USB_DRIVER 1
#define SD_DRIVER 2 // for SD cards

struct dstat
{
  int port;
  int class;
  int protocol;
  char *name;
};

struct blkdev
{
  struct spinlock lock;
  short blocks[64];
  int portno;
  char *name;
};

struct blkbuf
{
  void *blocks;
  size_t size;
  struct blkbuf *next;
  struct blkbuf *prev;
};

struct cmpdev
{
  int ports[4];
  struct blkbuf *buffer;
  int class;
  int sclass;
};

struct nulldev
{
  char *mem;
  size_t size;
  size_t flags;
};

struct rqueue
{
  struct rqueue* next;
  struct rqueue* prev;
  void* context;
  unsigned long time;
};

struct devdrv
{ // device driver
  int type, num;
  char isPCI;
  void (*hnd)(struct devdrv *, struct regs *);
  char (*getc)(struct devdrv *);
  void (*putc)(struct devdrv *, char);
  void (*init)(struct devdrv *);
};

int nullget(struct nulldev dev)
{ // returns EOF
  return (-1);
}

void nullset(struct nulldev dev, char *x)
{
  memcpy(dev.mem, x, dev.size);
}

uint16_t cmpread(struct cmpdev u, int no)
{
  if (no > 4)
    return 0;

  return inports(u.ports[no]);
}

void *ioalloc(size_t port, size_t bytes)
{
  int *u = (int *)pgalloc(bytes);
  *(u - 1) = inportb(port);
  return u;
}

struct blkdev *blkget(int fd)
{
  struct blkdev *u = TALLOC(struct blkdev);
  u->lock.locked = 0;
  u->portno = fd - 0x1C0000;
  u->blocks[63] = 0;
  return u;
}

void pushio(struct circbuf *x, int port)
{ // reads from port and adds it to the circular buffer
  char t = inb(port);
  pushcbuf(x, t);
}

void popio(struct circbuf *x)
{
  erscbuf(x);
}

void cpyio(struct circbuf *x, int port, int size)
{
  int a;
  for (a = 0; a < size; a++)
    pushio(x, port);
}

void blkcpy(struct blkbuf *u, struct blkdev *i)
{
  memcpy((short *)i->blocks, (short *)u->blocks, 63);
}

void blkcpys(struct blkbuf *u, struct blkdev *devs[])
{
  int i = 0, j = 0;
  while (i < u->size)
  {
    blkcpy(u, devs[j]);
    i += 64;
    j++;
  }
}

void blklock(struct blkdev *u)
{
  acquire(&(u->lock));
}

void blkunlock(struct blkdev *u)
{
  release(&(u->lock));
}

struct blknode
{
  struct blkdev *dev;
  struct file root; // adding a filesystem
  int active;
};

char *readio(int p)
{
  char *buf = alloc(0, 64);
  insl(p, buf, 63);
  return buf;
}

int writeio(char *buf, int p)
{
  if (buf != 0)
  {
    outsl(p, buf, 63);
    memset(buf, 0, 63); // clears the buffer
    return 1;
  }
  return -1;
}

struct bldev *blkcreat(char *name)
{
  struct blkdev *u;
  u->name = name;
  int i;
  for (i = 0; i < 64; i++)
    u->blocks[i] = 0;
  return u;
}

void blkfree(struct blkdev *_Block)
{
  _Block->portno = 0;
  int i;
  memset(_Block->blocks, 0, 63);
  free(_Block);
}

void blkclose(struct blknode _Node)
{
  int i;
  for (i = 0; i < 64; i++)
    _Node.dev->blocks[i] = 0;
  _Node.active = 0;
}

void blkopen(struct blknode _Node, int flags)
{
  _Node.root.flags = flags;
  _Node.root.parent = (struct file *)0;
}

char *blkread(struct blknode _Node)
{
  if ((_Node.root.flags & F_READ) && _Node.active)
    return readio(_Node.dev->portno);
}

char *blkreads(struct blkdev *_Block)
{
  if (_Block != 0 && _Block->lock.locked != 1)
    return readio(_Block->portno);
}

void blkwait(struct blknode _Node)
{
  while (_Node.active)
    ;
}

void blkwrite(struct blknode _Node, char *bytes)
{
  if ((_Node.root.flags & F_READ) && _Node.active)
    writeio(bytes, _Node.dev->portno);
}

void blkwrites(struct blkdev *_Block, char *bytes)
{
  if (_Block->lock.locked != 1)
    writeio(bytes, _Block->portno);
}
