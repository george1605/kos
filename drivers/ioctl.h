#pragma once
#include "../lib.c"
#include "../port.h"
#include "../tty.c"
#include "../fs.h"
#include "modules/hashmap.h"
//ported from Linux
typedef uint8_t cc_t;

struct termios2 {
   size_t iflag;       /* input mode flags */
   size_t oflag;       /* output mode flags */
   size_t cflag;       /* control mode flags */
   size_t lflag;       /* local mode flags */
   cc_t line;            /* line discipline */
   cc_t cc[19];        /* control characters */
   size_t ispeed;       /* input speed */
   size_t ospeed;       /* output speed */
};

#define IO_NONE 0
#define IO_READ 2
#define IO_WRITE 1
#define IO_NRSHIFT 0
#define IO_TYPESHIFT 8
#define IO_SIZESHIFT 16
#define IO_DIRSHIFT 30
#define _IOC_NRMASK	((1 << 8)-1)
#define _IOC_TYPEMASK	((1 << 8)-1)
#define _IOC_SIZEMASK	((1 << 14)-1)
#define _IOC_DIRMASK	((1 << 2)-1)
#define IOC(dir,type,nr,size) \
        (((dir)  << IO_DIRSHIFT) | \
        ((type) << IO_TYPESHIFT) | \
        ((nr)   << IO_NRSHIFT) | \
        ((size) << IO_SIZESHIFT))

#define IOC_TYPECHECK(s) sizeof(s)
#define IO(type,nr)               IOC(IO_NONE,(type),(nr),0)
#define IOR(type,nr,size)         IOC(IO_READ,(type),(nr),(IOC_TYPECHECK(size)))
#define IOW(type,nr,size)         IOC(IO_WRITE,(type),(nr),(IOC_TYPECHECK(size)))
#define IOWR(type,nr,size)        IOC(IO_READ|IO_WRITE,(type),(nr),(IOC_TYPECHECK(size)))
#define _IOC_DIR(x) (x >> IO_DIRSHIFT) & _IOC_DIRMASK
#define _IOC_TYPE(x) (x >> IO_TYPESHIFT) & _IOC_TYPEMASK
#define _IOC_NR(x) (x >> IO_NRSHIFT) & _IOC_NRMASK
#define _IOC_SIZE(x) (x >> IO_SIZESHIFT) & _IOC_SIZEMASK

#define TCGETS           0x5401
#define TCSETS           0x5402
#define TCSETSW          0x5403
#define TCSETSF          0x5404
#define TCGETA           0x5405
#define TCSETA           0x5406
#define TCSETAW          0x5407
#define TCSETAF          0x5408
#define TCSBRK           0x5409
#define TCXONC           0x540A
#define TCFLSH           0x540B
#define TIOCEXCL         0x540C
#define TIOCNXCL         0x540D
#define TIOCSCTTY        0x540E
#define TIOCGPGRP        0x540F
#define TIOCSPGRP        0x5410
#define TIOCOUTQ         0x5411
#define TIOCSTI          0x5412
#define TIOCGWINSZ       0x5413
#define TIOCSWINSZ       0x5414
#define TIOCMGET         0x5415
#define TIOCMBIS         0x5416
#define TIOCMBIC         0x5417
#define TIOCMSET         0x5418
#define TIOCGSOFTCAR     0x5419
#define TIOCSSOFTCAR     0x541A
#define FIONREAD         0x541B

#define TCGETS2          IOR('T', 0x2A, struct termios2)
#define TCSETS2          IOW('T', 0x2B, struct termios2)
#define TCSETSW2         IOW('T', 0x2C, struct termios2)
#define TCSETSF2         IOW('T', 0x2D, struct termios2)

#define DEVGETC          IOR('D', 0x01, uint8_t) /* Generic Device IOCTL - Used by Kernel */
#define DEVSETC          IOW('D', 0x02, uint8_t)
#define DEVRESET         IOW('D', 0x03, uint8_t)
#define DEVSETIRQ        IOW('D', 0x04, void*)
#define DEVINIT          IOW('D', 0x05, uint8_t)
#define DEVSTAT          IOR('D', 0x06, struct stat*)

#define FBSEL            IOW('F', 0x01, uint8_t)
#define FBSETP           IOW('F', 0x02, uint32_t)
#define FBGETP           IOR('F', 0x03, uint32_t)
#define FBGETWD          IOR('F', 0x04, uint16_t*)
#define FBGETHG          IOR('F', 0x05, uint16_t*)

#define KEY_SEND	 231	
#define KEY_REPLY	 232	
#define KEY_FORWARDMAIL	 233	
#define KEY_SAVE	 234	
#define KEY_DOCUMENTS	 235

#define KEY_BATTERY	 236
#define KEY_BLUETOOTH	 237
#define KEY_WLAN	 238
#define KEY_UWB		 239

#define KEY_UNKNOWN	 240

#define KEY_VIDEO_NEXT	 241	
#define KEY_VIDEO_PREV	 242
#define KEY_DVD		 0x185	/* Media Select DVD */
#define KEY_AUX		 0x186
#define KEY_MP3		 0x187
#define KEY_AUDIO	 0x188
#define KEY_VIDEO	 0x189	

#define LED_NUML	 0x00
#define LED_CAPSL	 0x01
#define LED_SCROLLL	 0x02

struct inputev { /* input event */
  int type;
  int time;
  int code;
  int value;
};

#define BUS_PCI 0x1
#define BUS_DMA 0x2 
#define BUS_USB 0x4 // usb keyboard, mouse etc.
#define BUS_IO  0x8 // io ports

struct deviceinfo {
  int type; // char or block
  size_t blocksize; // for block devs
  int irq_no; // irq number (if needed)
  int bus_type;
  size_t bus_num;
};

int map_device(int fd, ext2_gen_device* device)
{
  struct proc* p = myproc();
  fd = fdremap(fd, p->ofiles);
  if(p->ofiles[fd].extra != NULL_PTR)
    return -1; // already been used!
  memcpy(p->ofiles[fd].extra, device, sizeof(ext2_gen_device));
  p->ofiles[fd].flags = F_DEV;
  return fd;
}

void find_device(int fd, ext2_gen_device* device)
{
  struct proc* p = myproc();
  memcpy(device, p->ofiles[fd].extra, sizeof(ext2_gen_device));
}

int is_device(int fd)
{
  struct proc* p = myproc();
  return p->ofiles[fd].flags & F_DEV;
}

static void dev_setup_pci(ext2_gen_device* dev, uint32_t device)
{
  dev->priv = kalloc(sizeof(struct deviceinfo), KERN_MEM);
  struct deviceinfo* info = (struct deviceinfo*)dev->priv;
  info->irq_no = pci_get_interrupt(device);
  info->bus_num = device;
  info->type = INODE_TYPE_CHAR_DEV;
  info->bus_type = BUS_PCI;
}

static void dev_setup_io(ext2_gen_device* dev, uint16_t port)
{
  dev->priv = kalloc(sizeof(struct deviceinfo), KERN_MEM);
  struct deviceinfo* info = (struct deviceinfo*)dev->priv;
  info->irq_no = 32;
  info->bus_num = port;
  info->type = INODE_TYPE_CHAR_DEV;
  info->bus_type = BUS_IO;
}

void dev_free(ext2_gen_device* dev)
{
  free(dev->priv);
}

static uint8_t dev_getc(int fd)
{
  char charb[2]; // for safety measures
  ext2_gen_device device;
  find_device(fd, &device);
  device.read((uint8_t*)charb, device.offset, 1, &device);
  return charb[0]; 
}

static void dev_putc(int fd, uint8_t chars)
{
  ext2_gen_device device;
  find_device(fd, &device);
  char charp[2] = {chars, 0};
  device.write((uint8_t*)charp, device.offset++, 1, &device);
}

static void dev_set_irq(int fd, void(*func)(struct regs*))
{
  ext2_gen_device device;
  find_device(fd, &device);
  struct deviceinfo* info = (struct deviceinfo*)device.priv;
  irq_install_handler(info->irq_no, func);
}

static void dev_stat(int fd, struct stat* stat)
{
  ext2_gen_device device;
  find_device(fd, &device);
  struct deviceinfo* info = (struct deviceinfo*)device.priv;
  stat->st_dev = device.dev_no;
  stat->st_blksize = info->blocksize;
  stat->st_blocks = 0;
} 

void dev_init(int fd, size_t args)
{
  ext2_gen_device* dev = (ext2_gen_device*)myproc()->ofiles[fd].extra;
  switch((args >> 32) & 0xFF)
  {
  case 1:
    dev_setup_io(dev, args);
    break;
  case 2:
    dev_setup_pci(dev, args);
    break;
  }
}

static void dev_ctl(int fd, size_t cmd, size_t args)
{
  switch(cmd)
  {
  case DEVGETC:
    *(uint8_t*)args = dev_getc(fd);
  case DEVSETC:
    dev_putc(fd, args & 0xff);
  case DEVSETIRQ:
    dev_set_irq(fd, (void(*)(struct regs*))args);
  case DEVSTAT:
    dev_stat(fd, (struct stat*)args);
  case DEVINIT:
    dev_init(fd, args);
  }
}

struct _ioctl {
  struct spinlock i_lock;
  hashtable i_map; 
} ioctlmap;

void ioctl(int fd, size_t cmd, size_t arg)
{
  if(is_tty(fd))
    ttyctl(fd - TTYBASE, cmd - 0x5400);
  else if(is_device(fd) && _IOC_TYPE(cmd) == 'D')
    dev_ctl(fd, cmd, arg);
  else {
    void* func = hashmap_get(&ioctlmap.i_map, (void*)(_IOC_TYPE(cmd)));
    ((void(*)(int, size_t, size_t))func)(fd, cmd, arg);
  }
}