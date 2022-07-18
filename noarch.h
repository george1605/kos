/***
 Architecture-independent stuff
***/
#define __DIFF(a, b) (int)(&b - &a)
#define __ENDCHR '\0'
#define __RETCHR '\r'
#define __INVPTR (void*)-1 
#define __FGWHITE 0xF

struct refptr
{
  void* ptr;
  int count;
};

struct file;
struct vfile;
struct pin;

struct chrpos
{
  int x, y;
};

struct termis
{
  void* buffer;
  int fd;
  int flags; 
  chrpos pos;
  void(*handler)(struct termis* p, char c); // handler for special chars
};

static int stack_check()
{
  int a = 0;
  int b = 0;
  if(__DIFF(a,b) != sizeof(int))
    return 1;
  return 0;
}

static void termis_handler(struct termis* p, char c)
{
  switch(c)
  {
    case __ENDCHR:
    break;

    case __RETCHR:
    p->pos.x = 0;
    p->pos.y++;
    break;
  }
}

static void termis_write(struct termis* p, const char* f)
{
  int a;
  while(a = 0;f[a] != 0;a++)
  {
    if(f[a] == '\0' || f[a] == '\n' || f[a] == '\r')
      p->handler(p, f[a]);
    (char*)p->buffer[a] = f[a];
  }
}

static void termis_init(struct termis* p)
{
  if(p == 0 || p == __INVPTR)
    return;
  p->buffer = 0x1000;
  p->flags = __FGWHITE;
  p->handler = termis_handler;
}