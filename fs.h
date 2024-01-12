#pragma once
#include "ide.c"
#include "ata.h"
#include "mem.h"
#include "lib.c"
#include "./modules/ext2.h"
#define F_NEW 1
#define F_READ 2
#define F_WRITE 4
#define F_RDWR 8
#define F_DIR 32
#define F_DEV 64 /* device file */
#define F_PIPE 128
#define F_EXEC 0xFF
#define F_VIRT 0x100 /* generic vfile - may be F_DEV (or not) */
#define NDEV 16
#define NDIRECT 12
#define NINDIRECT (512 / sizeof(size_t))
#define I_BUSY 0x1
#define I_VALID 0x2

#define ERR_BADDISK 0xA9
#define ERR_BIGLOG 0xAA
#define ERR_OUTTRANS 0xAB
ext2_gen_device* fs_dev;

struct buf* ata_read(size_t dev, size_t sector)
{
  struct buf* u = (struct buf*)kalloc(sizeof(struct buf), KERN_MEM);
  struct atadev* tdev = (dev == 0)? &ata_primary_master : &ata_primary_slave;
  char* x = ata_read_sector(tdev,sector);
  memcpy(u->data,x,512);
  sfree(x);
  return u;
}

// reads (start - end + 1) / 512 sectors
void ata_reads(size_t dev, uint8_t* s, size_t start, size_t end)
{
  size_t len = end - start + 1, sz = 0;
  size_t first = start / 512;
  size_t slen = len / 512;
  struct atadev* tdev = (dev == 0)? &ata_primary_master : &ata_primary_slave;
  for(sz = 0;sz < slen;sz++) {
      ata_kread_sector(s, tdev, sz);
      s += SECTOR_SIZE;
  }
}

char *cwdpath;
size_t balloc(size_t dev)
{
  struct buf *bp;
  int ind = 14, found = 0;
  while(1)
  {
    bp = breads(dev, ind);
    if (bcasti(bp, 0) != 0xdeadbeef)
      return ind;
    ind++;
  }
}

struct dinode
{
  short type;
  short major;
  short minor;
  short nlink;
  size_t size;
  size_t addrs[NDIRECT + 1];
};

struct file
{
  char *name;
  char *dname;
  struct file *parent;
  struct file **children;
  int chid;
  int chno;
  int fd;
  int flags;
  char open;
  void* extra;
} root;

struct ftable {
  struct file* files;
  int fcnt;
  struct spinlock lock;
};

void setuproot()
{
  if(fs_dev == NULL_PTR)
    return;
  
  filesystem* fs = fs_dev->fs;
  fs->touch("/home", fs_dev, NULL_PTR);
  root.parent = NULL_PTR;
}

void faddtab(struct file i, struct ftable u){ // adds the file to the FILETABLE
  acquire(&u.lock);
  u.files[++u.fcnt] = i;
  release(&u.lock);
}

struct filelock
{
  struct spinlock lock;
  struct file file;
};

struct ftable loadtbl(int ide)
{
  struct buf* b = TALLOC(struct buf);
  b->blockno = 2;
  b->flags = B_VALID;
  bread(b, 0);
  struct ftable ft;
  memcpy(&ft, b->data, 64);
  free(b);
  return ft;
}

int savetbl(int ide, struct ftable* ft)
{
  if(ft == NULL_PTR) return -1;
  struct buf *b = TALLOC(struct buf);
  b->blockno = 2;
  b->flags = B_DIRTY;
  memcpy(b->data, ft, sizeof(*ft));
  int p = bwrite(b, 0);
  free(b);
  return p;
}

void unveil(const char *path, int perm)
{
  struct ftable ft = loadtbl(0);
  int found = 0, c = 0;

  while(!found)
  {
    if(strcmp(ft.files[c].name, path) == 0)
      ft.files[c].flags = perm, found = 1;

    c++;
  }

  if(!found)
    perror("File not found!");

  if(savetbl(0, &ft) == -1)
    perror("FileTable could not be saved!");
}

// file name like "1:/what.txt"(drive D in windows)
struct file findfile(const char* name)
{
  int dsk = 0, c = 0; // by default the first file
  if(name[1] == ':') {
    dsk = (name[0] - '0');
    name += 2;
  }
  struct ftable ft = loadtbl(dsk);
  for(c = 0;c < ft.fcnt;c++) {
    if(strcmp(ft.files[c].name, (char*)name) == 0)
      return ft.files[c];
  }
  return (struct file){.name = (char*)NULL_PTR, .fd = -1};
}

int fperms(struct file file, int wanted)
{
  return (file.flags & wanted);
}

void free_fd(int fd)
{

}

struct fopener // useful for the 'Open With' actions
{
  char* name;
  char* path;
  void(*open)(struct file);
};

struct filelock flock(struct file i)
{
  struct filelock u;
  u.file = i;
  acquire(&u.lock);
  return u;
}

char *getpath(char *bytes)
{
  char *u = (char *)alloc(0, 256);
  int a, b = 0;
  for (a = 0; cwdpath[a] != 0; a++)
  {
    u[a] = cwdpath[a];
  }
  u[++a] = '/';
  while (bytes[a] != 0)
  {
    u[++a] = bytes[b++];
  }
  return u;
}

void nmdir(char* name)
{
  int dir = 0;
  if(name[0] >= '0')
      dir = name[0] - '0';
  ext2_gen_device* dev = ext2_from_ata(dir);
  ext2_read_root_directory(name + 2, dev, (ext2_priv_data*)NULL_PTR);
}

#define VER_WRITE F_RDWR
#define VER_READ F_READ

int access_ok(void* ptr, int perm, int size) // TODO here
{
  if(ptr == NULL_PTR || ptr < 0xFFFF) return;
  if(perm == VER_READ)
  {
    if(*(int*)ptr == 0xDEADC0DE || *(int*)(ptr + size) == 0xDEADC0DE)
      return 1;
    return 0;
  }
  return 0;
}

typedef unsigned long long fileptr_t;

typedef struct
{
    int disk;
    fileptr_t ptr;
} fsptr_t;


fileptr_t getfptr(int fd)
{
  struct ftable n = loadtbl(0);
  struct file n;

  for(int i = 0;i < n.fcnt;i++)
    if(n.files[i].fd == fd)
      return (fileptr_t)n.files[i].fd; // TODO: Add file Pointers
}

void setfptr(int fd, fileptr_t x)
{
  char ptr[512] = {0x80, 0xb1};
  int _f = fd;

  memcpy(ptr + 3, &_f, sizeof(_f));
  ata_write_sector(&ata_primary_master, x / 512 , ptr);
}

void funlock(struct filelock u)
{
  release(&u.lock);
}

size_t fislock(struct filelock u)
{
  return (u.lock.locked);
}

struct fileops
{
  void (*write)(struct file a, char *data);
  void (*open)(char *a, int mode);
  void (*close)(struct file a);
  void (*copy)(struct file a, char *location);
} fops;

struct file fcreat(char* name)
{
  struct file f;
  f.name = name;
  f.parent = &root;
  struct farea* k = falloc(lastsect, 1, 128);
  f.open = 1;
  return f;
}

struct inode
{
  size_t dev;
  size_t inum;
  int ref;
  int flags;
  short type;
  short major;
  short minor;
  short nlink;
  size_t size;
  size_t addrs[NDIRECT + 1];
};

int lastinum = 0;

struct dentry
{
  char *name;
  struct inode ind;
};

struct dentry dennew(char *dname, int dev)
{
  struct dentry u;
  u.name = dname;
  u.ind.dev = dev;
  u.ind.inum = ++lastinum;
  u.ind.flags = F_NEW;
  return u;
}

struct logheader
{
  int n;
  int block[30];
};

struct log
{
  struct spinlock lock;
  int start;
  int size;
  int outstanding;
  int committing;
  int dev;
  struct logheader lh;
} log;

void waitlog(struct log u)
{
  if (u.lock.locked)
  {
    prswap(&tproc);
  }
}

struct cache
{
  struct spinlock lock;
  struct buf buf[30];
  struct buf head;
} bcache;

void cacheflush(struct cache *chc)
{
  release(&(chc->lock));
  int i = 0;
  chc->head.flags |= B_NONE; // B_NONE means ignore the buffer
  while (i < 30)
  {
    chc->buf[i].flags |= B_NONE;
    i++;
  }
}

void logflush(struct logheader *hdr)
{
}

struct superblock
{
  size_t size;
  size_t nblocks;
  size_t ninodes;
  size_t nlog;
  size_t logstart;
  size_t inodestart;
  size_t bmapstart;
} sb;

struct extblock 
{
  size_t ninodes;
  size_t nblocks;
  size_t nres;
  size_t nfblocks;
  size_t nfinodes;
};

void ext_initsb(struct extblock *u)
{
  if(u != 0)
  {
    u->ninodes = 0;
    u->nblocks = 0;
    u->nres = 2;
    u->nfblocks = 32;
    u->nfinodes = 17;
  }
}

void initsb(struct superblock *u)
{ /* creates a default superblock */
  if (u != 0)
  {
    u->ninodes = 0;
    u->nlog = 0;
    u->nblocks = 0;
    u->logstart = 6;
    u->inodestart = 10;
    u->bmapstart = 14;
  }
}

void addlog(struct log lg, struct superblock *u)
{
  if (u != 0)
    u->nlog++;
}

void rmlog(struct log lg, struct superblock *u)
{
  if (u->nlog > 0)
    u->nlog--;
}

void readsb(int dev, struct superblock *u)
{
  struct buf *b;
  b->blockno = 1;
  bread(b, dev);
  memmove(u, b->data, sizeof(struct superblock));
  brelse(b);
}

void writesb(int dev, struct superblock *u)
{
  struct buf *b;
  b->blockno = 1;
  bwrite(b, dev);
  memmove(u, b->data, sizeof(struct superblock));
  brelse(b);
}

void freesb(struct superblock *u)
{
  u->nblocks = 0;
  u->inodestart = 0;
  u->logstart = 0;
  u->ninodes = 0;
  u->nlog = 0;
  u->size = 0;
}

int checksb(int dev)
{ // checks for valid sb
  struct superblock *u = (0x220000 + sizeof(struct superblock) * dev);
  readsb(dev, u);
  int t = (u->logstart < 6 || u->inodestart < 10 || u->bmapstart < 14);
  freesb(u);
  return (!t);
}

void checkfs()
{
  if (checksb(0) == 0)
    raise(ERR_BADDISK);
}

void exitlog(struct log u)
{
  release(&u.lock);
}

struct log initlog(int dev)
{
  struct log u;
  struct superblock sb;
  initlock(&u.lock, "log");
  readsb(dev, &sb);
  u.start = sb.logstart;
  u.size = sb.nlog;
  u.dev = dev;
  exitlog(u);
  return u;
}

struct superblock *getsb(int dev)
{ // allocates and reads an superblock
  struct superblock *u = kalloc(sizeof(struct superblock), KERN_MEM);
  readsb(dev, u);
  return u;
}

void addnode(struct inode node, struct superblock *u)
{
  if (u != 0)
    u->ninodes++;
}

void rmnode(struct inode node, struct superblock *u)
{
  if (u != 0)
    if (u->ninodes > 0)
      u->ninodes--;
}

void chinit()
{
  initlock(&bcache.lock, "bcache");
}

void ilock(struct inode *u)
{
  struct spinlock t;
  initlock(&t, "inode");
  u->flags |= I_BUSY;
  acquire(&t);
}

void iunlock(struct inode *u)
{
  u->flags &= I_BUSY;
  u->flags |= I_VALID;
}

struct stat
{
  unsigned long dev;
  unsigned long ino;
  unsigned int mode;
  unsigned int nlink;
  unsigned int uid;
  unsigned int gid;
  unsigned long rdev;
};

size_t fsize(struct file u)
{
  return 0;
}

struct stat fstat(struct file f)
{
  struct stat i;
  i.uid = 1;
  return i;
}

int isdir(struct file u)
{
  return (u.flags & F_DIR);
}

struct file mkdir(char *dname, struct file *parent)
{
  struct file _Dir;
  _Dir.parent = parent;
  _Dir.name = dname;
  _Dir.flags = (F_RDWR | F_DIR);
  parent->children[++parent->chno] = (&_Dir);
  return _Dir;
}

void mkfs()
{
  struct file *_ROOT = &root;
  _ROOT->children = (struct file **)0x100FF00;
  mkdir("/home/lib", _ROOT);
  mkdir("/home/user", _ROOT);
  mkdir("/home/bin", _ROOT);
  mkdir("/home/etc", _ROOT);
  mkdir("/home/sys", _ROOT);
}

void rmdir(struct file u)
{
  u.parent = 0;
  u.fd = 0;
}


int _getfd(void* data, int type){
  int fd = 0;
  if(type == F_DEV){
    fd = *(int*)data + 0x1C0000;
    return fd;
  }
  return fd;
}

int _open(char* filename, int perms)
{
  if(!fs_dev)
    fs_dev = ext2_from_ata(0);
  ext2_inode inode;
  ext2_priv_data data;
  data.blocksize = 1; // 1 sector
  ext2_find_file_inode(filename, &inode, fs_dev, &data);
  return inode.gen_no;
}

int _read(int fd, struct buf *buffer, size_t buffer_sz)
{
  if (buffer != 0)
  {
    if (fd == 0)
    {
      memcpy((void *)buffer->data, BUFFER, buffer_sz); // copies from the text buffer
      return 0;
    }
    if (fd == 1)
    {
      memcpy((void *)buffer->data, kbdbuf, buffer_sz); // copies from kbd buffer
      return 0;
    }
    if (fd > 0x1C0000 && fd < 0x1CFFFF && (buffer->flags & 0x8))
    {
      insl(fd - 0x1C0000, buffer->data, buffer_sz); // 
    }
    buffer->blockno = (int)fd / BSIZE;
    bread(buffer, buffer->dev);
  }
  return 0;
}

int _write(int fd, struct buf *buffer, size_t buffer_sz)
{
  if (buffer != 0)
  {
    if (fd == 0)
    {
      memcpy(BUFFER, (void *)buffer->data, buffer_sz);
      return 0;
    }
    if (fd == 1)
    {
      memcpy(kbdbuf, (void *)buffer->data, buffer_sz);
      return 0;
    }
    if (fd > 0x1C0000 && fd < 0x1CFFFF && (buffer->flags & 0x8))
    {
      outsl(fd - 0x1C0000, buffer->data, buffer_sz);
    }
    buffer->blockno = (int)fd / BSIZE;
    bwrite(buffer, buffer->dev);
  }
  return 0;
}

char* _readx(int fd,int flags) // reads 512 bytes
{
  struct buf* b = kalloc(sizeof(struct buf*),KERN_MEM);
  b->flags = B_VALID | flags;
  b->dev = 0;
  _read(fd,b,512);
  return (char*)b->data;
}

void qbuf(struct buf* b, char* p){ // writes the data from queue to p
  struct buf* aux;
  for(aux = b;aux != 0 && p != 0;aux = aux->qnext){
    memcpy(p,aux->data,512);
    p += 512;
  }
}

struct buf *fbuf(struct file k)
{
  struct buf *u = TALLOC(struct buf);
  u->blockno = (int)(k.fd / 512);
  u->off = k.fd % 512;
  if (k.flags == F_READ)
    u->flags = B_VALID;
  else if (k.flags == F_WRITE)
    u->flags = B_DIRTY;
  else
    u->flags = (B_VALID | B_DIRTY);
}

void log_write(struct buf *b)
{
  int i, err;

  if (log.lh.n >= 30 || log.lh.n >= log.size - 1)
    err = ERR_BIGLOG;
  if (log.outstanding < 1)
    err = ERR_OUTTRANS;

  acquire(&log.lock);
  for (i = 0; i < log.lh.n; i++)
  {
    if (log.lh.block[i] == b->blockno) // log absorbtion
      break;
  }
  log.lh.block[i] = b->blockno;
  if (i == log.lh.n)
    log.lh.n++;
  b->flags |= B_DIRTY; // prevent eviction
  release(&log.lock);
}

void rmfile(struct file u)
{
  struct buf *k = fbuf(u);
  bclr(k);
  k->flags = B_DIRTY;
  iderw(k); // clears the data from DISK
  brelse(k);
  u.fd = u.parent = 0; // unlink()
}

void rrmdir(struct file dir)
{
  int i = 0;
  if (dir.children != 0)
    while (dir.children[i] != 0)
    {
      rmfile(*dir.children[i]);
      i++;
    }
  rmdir(dir);
}

#define IPB (512 / sizeof(struct dinode))
#define IBLOCK(i, sb) ((i) / IPB + sb.inodestart)

void iupdate(struct inode *ip)
{
  struct buf *bp;
  struct dinode *dip;

  bp = breads(ip->dev, IBLOCK(ip->inum, sb));
  dip = (struct dinode *)(bp->data + ip->inum % IPB);
  dip->type = ip->type;
  dip->major = ip->major;
  dip->minor = ip->minor;
  dip->nlink = ip->nlink;
  dip->size = ip->size;
  memmove(dip->addrs, ip->addrs, sizeof(ip->addrs));
  log_write(bp);
  brelse(bp);
}

void rdfile(struct file u)
{
  if (u.fd != 0)
  {
    struct buf *t = fbuf(u);
    iderw(t);
  }
}

struct file opfile(char *fname)
{
  struct file u;
  u.fd = (int)dalloc(64);
  u.name = fname;
  u.flags = F_NEW;
  return u;
}

void fs_init()
{
  ideinit();
  idenew();
  ata_init(); // add all of them here (flash_init also)
  fs_dev = (ext2_gen_device*)kalloc(sizeof(ext2_gen_device), KERN_MEM);
  ext2_probe(fs_dev);
  setuproot();
}

struct drive
{
  char *name;
  struct file *droot;
  int size;
};

void renamedr(struct file t, const char *name)
{
  t.name = (char *)name;
}

struct file fialloc(int bytes, struct file *parent)
{
  struct file u;
  if (parent == 0)
    u.parent = &root;

  u.name = (char*)"/home/null";
  u.flags = F_NEW;
  return u;
}

void fifree(struct file u)
{
  u.parent = 0;
}

static size_t bmap(struct inode *ip, size_t bn)
{
  size_t addr, *a;
  struct buf *bp;

  if (bn < NDIRECT)
  {
    if ((addr = ip->addrs[bn]) == 0)
      ip->addrs[bn] = addr = balloc(ip->dev);
    return addr;
  }
  bn -= NDIRECT;

  if (bn < NINDIRECT)
  {
    // Load indirect block, allocating if necessary.
    if ((addr = ip->addrs[NDIRECT]) == 0)
      ip->addrs[NDIRECT] = addr = balloc(ip->dev);
    bp = breads(ip->dev, addr);
    a = (size_t *)bp->data;
    if ((addr = a[bn]) == 0)
    {
      a[bn] = addr = balloc(ip->dev);
      log_write(bp);
    }
    brelse(bp);
    return addr;
  }
}

void bzero(int dev, int bno)
{
  struct buf *bp;

  bp = breads(dev, bno);
  memset(bp->data, 0, 512);
  log_write(bp);
  brelse(bp);
}

struct vfile;
extern void vfsread(struct vfile vf, char* buf, size_t size);

int readi(struct inode *ip, char *dst, size_t off, size_t n)
{
  size_t tot, m;
  struct buf *bp;

  if (ip->type == F_DEV)
  {
    struct vfile vf;
    vf.drno = ip->dev;
    vf.status = ip->flags;
    vf.fd = ip->inum;
    vfsread(vf, dst, n);
    return 0;
  }

  if (off > ip->size || off + n < off)
    return -1;
  if (off + n > ip->size)
    n = ip->size - off;

  for (tot = 0; tot < n; tot += m, off += m, dst += m)
  {
    bp = breads(ip->dev, bmap(ip, off / BSIZE));
    m = min(n - tot, BSIZE - off % BSIZE);
    memmove(dst, bp->data + off % BSIZE, m);
    brelse(bp);
  }
  return n;
}

void fclose(struct file* f)
{
  f->open = 0;
  f->parent = NULL_PTR;
  idewait(0);
}

void fsdump()
{
  if(!fs_dev)
  {
    printf("No info to display\n");
    return;
  }
  printf("Sectors read & written: %i", fs_dev->offset); // not nec. an offset, just a counter
}

struct fat_diren
{
  char name[128];
  uint8_t flags;
  uint32_t first_cluster;
  size_t size;
};

struct fat_dir
{
  struct fat_diren entry;
};

#define FAT_TABLE unsigned char fat_tbl[512]