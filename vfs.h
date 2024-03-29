#pragma once
#include "kb.c"
#include "lib.c"
#include "floppy.h"
#include "smem.h"
#include "drivers/blockdev.h"

#define PORTBASE 0x1C0000
#define STR_OUT 0
#define STR_IN 1

#define STDIN 0
#define STDOUT 1
#define STDERR 2
#define DRVSTR 3 /* driver log */

struct vfile
{
  struct vfile *parent;
  int drno;
  size_t fd;
  char *name;
  void *mem;
  size_t size; // memory size
  int refcnt; // for link and unlink
  int status;
  void* ops;
} vroot;

struct vfileops
{
  void (*write)(struct vfile* a, char *data, size_t);
  void (*read)(struct vfile* a, char *data, size_t);
  struct vfile* (*open)(char *a, int mode);
  void (*close)(struct vfile* a);
  void (*copy)(struct vfile* a, char *location);
};

void vfsclose(struct vfile* vf)
{
  struct vfileops* fops = (struct vfileops*)vf->ops;
  if(fops->close != NULL_PTR)
    fops->close(vf);
  else {
    vf->status = ~F_VIRT;
  }
}

// parses the path and creates the "tree"
void vfsparse(struct vfile* head, char* path)
{
  int i = sizeof(struct vfile);
  struct vfile* vf = head;
  for(;path[i] != '\0';i++)
  {
    if(path[i] == '/') // unix path
    {
      
    }
  }
}

struct vfile sysvf[16];
struct vfile systty[16];

struct vfile* vfslink(struct vfile* vf, char* path)
{
  vf->refcnt++;
  struct vfile* newf = (struct vfile*)kalloc(sizeof(struct vfile), KERN_MEM);
  memcpy(newf, vf, sizeof(struct vfile));
  newf->name = strdup(path);
  newf->parent = vf; // if they have the same fd then they are linked, then not
  return newf;
}

void vfslink2(struct vfile* vfpar, struct vfile* vfchld)
{
  if(vfpar == NULL_PTR || vfchld == NULL_PTR) return;
  memcpy(vfchld, vfpar, sizeof(struct vfile)); // copies everything, even fd
  vfchld->parent = vfpar;
  vfpar->refcnt++;
}

int fdalloc()
{
  for(int i = 0;i < 16;i++)
    if(sysvf[i].name == NULL_PTR)
      return i;
  return -1;
}

void vfsunlink(struct vfile* chvf)
{
  struct vfile* p = chvf->parent;
  if(p->fd != chvf->fd) // they are not linked, are different files
    return;
  if(--p->refcnt <= 0)
  {
    
  }
  chvf->parent = (struct vfile*)NULL_PTR;
  chvf->fd = fdalloc(); // get a different fd
}

struct kmap getmap(struct vfile u)
{
  struct kmap i;
  i.physs = u.mem;
  i.physe = u.mem + u.size;
  i.virt = ((uint64_t)i.physs >> 8) + KERN_MEM;
  if ((uint64_t)u.mem < 0xA0000)
    i.perm = M_USED | M_STATIC;

  if ((uint64_t)u.mem > 0x300000)
    i.perm = M_READ;

  if ((uint64_t)u.mem >= KERN_MEM)
    i.perm = M_BOTH;
  return i;
}

struct vinode
{
  size_t dev;
  size_t inum;
  int ref;
  int flags;
  short type;
};

struct vinode vicreat(int fd, int flags)
{
  struct vinode k;
  k.flags = flags;
  if (k.flags & 64)
    k.dev = 0xA; // network
  else if (k.flags & 4)
    k.dev = 0x0; // memory
  else
    k.dev = fd - 0x1C0000; // device
  return k;
}

struct stream
{
  int sid;
  char *buffer;
  int flags;
  int fail;
  int pos;
};

struct locnet
{ // local network
  size_t *IPs;
  size_t devs;
  char *name;
};

struct vfile vfsmap(char *name, void *memory)
{
  struct vfile u;
  u.parent = &vroot;
  u.name = name;
  u.mem = memory;
  u.status = 0x4;
  return u;
}

struct vfile vfsdmap(char *name, struct blkdev i)
{
  struct vfile u;
  u.mem = NULL_PTR;
  u.fd = i.portno + PORTBASE;
  u.status = 0x8;
  u.parent = &vroot;
  u.name = name;
  return u;
}

void vfsset(struct vfile i, size_t flag)
{
  i.status |= flag;
}

void vfswrite(struct vfile u, char *str)
{
  if (u.mem != 0)
  {
    memcpy(u.mem, str, strlen(str));
  }
  else if (u.status & 0x8)
  {
    writeio(str, u.fd - PORTBASE);
  }
  else
  {
    // rtl8139_send_packet(u.mem, strlen(u.mem));
  }
}

int vfscheck(char *name)
{ // checks if it is a system virtual file
  int i;
  for (i = 0; i < 16; i++)
  {
    if (strcmp(sysvf[i].name, name) == 0)
      return i;
  }
  return -1;
}

int isnulldev(char *path)
{
  if (!strcmp(path, "/home/dev/null"))
    return 1;
  return 0;
}

// /dev/random (gives random numbers)
void randread(struct vfile* vf, char* bytes, size_t size)
{
  int sz = 0;
  while(size - sz > sizeof(int)) {
    ((int*)bytes)[sz] = rand();
    sz += sizeof(int);
  }
  while(sz < size)
    bytes[sz++] = rand() & 0xff; 
}

void randomdev()
{
  int fd = fdalloc();
  sysvf[fd].name = "/home/dev/rand";
  sysvf[fd].ops = kalloc(sizeof(struct vfileops), KERN_MEM);
  struct vfileops* ops = (struct vfileops*)sysvf[fd].ops;
  ops->write = NULL_PTR; // if the write func is null then cannot write to it(simple)
  ops->read = randread;
}

struct vfile vfsata(int dev, char *name)
{
  struct atadev *u = (dev == 0) ? &ata_primary_master : &ata_secondary_master;
  struct vfile i;
  i.drno = dev;
  i.fd = 0x1C0000 + u->data;
  i.status = 72; // 64 + 8
  i.mem = u->mem_buffer;
  i.name = strdup(name);
  return i;
}

void atamap()
{
  if (ata_primary_master.data != 0x1F0 || ata_primary_master.alt_status != 0x3F6)
    ata_init();

  sysvf[12] = vfsata(0, "/home/dev/hd0.dev");
  sysvf[13] = vfsata(1, "/home/dev/hd1.dev");
}

struct vfile vfssd(int n, char *name)
{
  struct vfile u;
  u.drno = n + 3;
  u.name = name;
  u.mem = kalloc(256, KERN_MEM); // 256 bytes of cache
  return u;
}

void sdmap()
{
  sysvf[14] = vfssd(0, "/home/dev/sd0.dev");
}

void vfsinit()
{
  struct blkdev i;
  i.portno = 0x3f8;
  sysvf[0] = vfsmap("/home/sys/text.dev", (void *)0xB8000);
  sysvf[1] = vfsmap("/home/sys/kbuf.dev", (void *)0x2C00FF);
  sysvf[2] = vfsmap("/home/sys/sb.info", (void *)0x220000);
  sysvf[3] = vfsmap("/home/sys/fs.info", (void *)0x100FF00);
  sysvf[4] = vfsmap("/home/sys/stack.info", _vm(10));
  sysvf[5] = vfsdmap("/home/dev/com1.dev", i);
  atamap();
  randomdev();
}

void vfsrem()
{
  int i;
  for (i = 0; i < 16; i++)
  {
    free((int*)(sysvf[i].mem));
    sysvf[i].fd = 0;
  }
}

void swrite(struct stream *u, char *str)
{
  int i;
  for (i = 0; str[i] != 0; str++)
  {
    u->buffer[i + u->pos] = str[i]; // copies to the buffer ( without memcpy )
  }
  u->pos += i;
}

void sseek(struct stream *u, int nseek)
{
  if (u->pos + nseek > 0)
    u->pos += nseek;
}

void sclose(struct stream *u)
{
  if (u != 0 && u->buffer != 0)
  {
    free(u->buffer);
  }
}

struct vfile procmap(char* path, struct proc p)
{
  if(path == NULL_PTR)
    path = "/home/proc/";

  struct vfile x;
  x.mem = p.stack;
  x.fd = p.pid  + 0xFFFF00;
  x.name = strcat(path, p.name);
  return x;
}

void procunmap(struct vfile vf)
{
  free(vf.name);
}

char *drname(int drno)
{
  char *name = kalloc(8, KERN_MEM);
  name[0] = 'd';
  name[1] = (drno + 'C');
  return name;
}

struct vfile vfsopen(char *name)
{
  int c;
  if(c = vfscheck(name) && c != -1)
    return sysvf[c];
  
  struct vfile v;
  v.name = name;
  return v;
}

struct buf* vfsread_k(struct vfile vf)
{
  struct buf* x = TALLOC(struct buf);
  _read(vf.fd, x, 512);
  return x;
}

int vfseof(struct buf* buf)
{
  if(buf->flags == B_NONE)
  {
    _read(buf->dev, buf, 512);
  }
  return strcon(buf->data, '\0');
}

void vfsread(struct vfile vf, char* buffer, size_t size)
{
  struct buf* x = kalloc(sizeof(struct buf));
  int sz = 0;
  while(sz < size)
  {
    _read(vf.fd, x, 512);
    if(vfseof(x))
      break;
  }
}

struct vfile rfs_open(char *name)
{
  struct vfile u;
  u.fd = (size_t)kalloc(128, KERN_MEM);
  u.name = name;
  u.status = 1;
  u.parent = (struct vfile *)0;
}

void rfs_write(struct vfile u, const char *a, int size)
{
  if (a != (char *)0 && size != 0)
    memcpy((char*)u.fd, a, size);
}

void* vmap(void* location, size_t size, size_t flags, struct vfile* file)
{
  if(location == NULL_PTR)
    location = kalloc(size, USER_MEM);
  
  void* virt = map_page(location, location + rand(), flags);
  rseed++; // increases the seed
  if(file != NULL_PTR)
  {
    vfsread(*file, (char*)virt, size);
  }
  return virt;
}

void* vmapx(void* phys, void* virt, size_t size, size_t flags, struct vfile* file)
{
  if(virt == NULL_PTR)
    return vmap(phys, size, flags, file);
  
  void* addr = map_page(phys, virt, flags);
  if(file != NULL_PTR)
  {
    vfsread(*file, (char*)virt, size);
  }
  return addr;
}

struct vfile* fmemopen_k(void* memory, size_t size, int prot)
{
  struct vfile* vf = kalloc(sizeof(struct vfile), KERN_MEM);
  if(memory == NULL_PTR)
    memory = kalloc(size, USER_MEM);
  vf->mem = memory;
  vf->size = size;
  vf->fd = fdalloc();
  vf->status = prot;
  return vf;
}