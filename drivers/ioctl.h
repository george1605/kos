#pragma once
#include "../lib.c"
#include "../port.h"
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

struct inputhnd {
  const char *name;
  void *private;
};