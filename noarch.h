/***
 Architecture-independent stuff
***/
#define __DIFF(a, b) (int)(&b - &a)
#define __ENDCHR '\0'
#define __RETCHR '\r'

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