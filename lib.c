#pragma once
#define NULL_PTR (void *)0
#define BASESP 0x3C00
#define PACKED __attribute__((packed))
#define ASMV(x) asm volatile(x)
#define CLI() asm("cli")
#define STI() asm("sti")
#define HALT() asm("hlt")
#define NOP() asm("nop")
#define PAUSE() asm volatile("pause")
#define MIN(a, b) (b > a) ? (a) : (b)
#define MAX(a, b) (b < a) ? (a) : (b)
#define SIZESTR(x) sizeof(struct x)
#define TOINT(chr) chr << 24
#define _POSIX 3
#define _ESGUS 1 /* Extended Standard for Graphical Unix Systems */
#define NCPU 32

#define NORMAL_VGA 0xffff
#define EXTENDED_VGA 0xfffe
#define ASK_VGA 0xfffd

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long size_t;
typedef uint16_t wchar_t;
typedef unsigned long long uint64_t;
typedef double f64;
typedef char* va_list;
size_t errno;

void *getframe()
{
  void *frame;
  asm("mov %%ebp,%0"
      : "=r"(frame));
  return frame;
}

uint32_t geteip() {
  uint32_t esp, eip;
  asm volatile ("mov %%esp, %0" : "=r"(esp));
  eip = *(uint32_t*)esp;
  return eip;
}

int invmod(int A, int B, int X) // ( A / B ) % X
{
  return ((B % X) / (A % X)) % X;
}

int ssqrt(int x) // slow sqrt
{
  int u;
  for(int u = 1; u < x/2; x++)
    if(u * u == x)
      return x;
  return (-1);
}

int imod2(int x, int y)
{
  while (x > y)
  {
    x -= y;
  }
  return x;
}

#ifdef _X86_
f64 sin(f64 x)
{
  f64 result;
  asm("fsin"
      : "=t"(result)
      : "0"(x));
  return result;
}
#endif

int overflow(size_t *ptr)
{ // checks for overflow
  long *i = (long *)ptr;
  if (*i < 0)
    return 1;
  return 0;
}

size_t abs(long long l) // out of the box support for long numbers
{
  if(l < 0)
    return -l;
  return l;
}

void *idalloc(size_t id)
{
  int *ptr = (int *)(id + 0x2C00000 + (id % 3)); // just to randomise a bit
  *(ptr - 1) = 0xdeadbeef;
  *ptr = id;
  return ptr;
}

void idfree(void *ptr)
{
  if (ptr < 2)
    return;

  int *k = ptr;
  *k = 0;
  *(k - 1) = 0;
}

int argsz(char **argv)
{ // gets the size of the argument
  int x = 0;
  if (argv != 0)
  {
    while (argv[x] != 0 && x < 100)
      x++;
  }
  return x;
}

uint8_t sum(uint8_t *addr, int len)
{
  int i, sum;

  sum = 0;
  for (i = 0; i < len; i++)
    sum += addr[i];
  return sum;
}

uint64_t fastpow(uint64_t base, uint64_t exp)
{
  if(exp == 0)
    return 1ULL;
  if(exp % 2 == 0) {
    uint64_t hlf = fastpow(base, exp/2);
    return hlf * hlf;
  } else {
    uint64_t hlf = fastpow(base, exp/2);
    return base * hlf * hlf;
  }
}

size_t strlen(char *u)
{
  int i = 0;
  if (u != 0)
  {
    while (*(u + i) != 0)
      i++;
  }
  return i;
}

static inline void memset(void *dst, unsigned char value, size_t n)
{
  uint8_t *d = (uint8_t*)dst;

  while (n-- > 0)
  {
    *d++ = value;
  }
}

static inline void memsetw(void *dst, size_t value, size_t n)
{
  size_t *d = (size_t*)dst;

  while (n-- > 0)
  {
    *d++ = value;
  }
}

static inline void *memcpy(void *dst, const void *src, size_t n)
{
  unsigned char *d = dst;
  const unsigned char *s = src;

  while (n-- > 0)
  {
    *d++ = *s++;
  }

  return d;
}

char* strcat(char* a1, char* a2)
{
  int y = strlen(a1), z = strlen(a2);
  char* x = (char*)alloc(0, y + z + 1);
  memcpy(x, a1, y);
  memcpy(x + y,  a2, strlen(a2));
  return x;
}

void write32(void *address, int value)
{
  (*(volatile int *)(address)) = value;
}

int read32(void *address)
{
  if (address != 0)
    return (*(volatile int *)(address));
}

void write16(void *address, int value)
{
  (*(volatile short *)(address)) = value;
}

int read16(void *address)
{
  if (address != 0)
    return (*(volatile short *)(address));
}

int min(int a, int b)
{
  return a * (a <= b) + b * (b < a);
}

int max(int a, int b)
{
  return a * (a >= b) + b * (b > a);
}

void swap(int a, int b)
{
  int aux = a;
  a = b;
  b = aux;
}

struct proc;

struct context
{
  int edi;
  int esi;
  int ebx;
  int ebp;
  int eip;
} ccon;

struct taskstate
{
  size_t esp0;
  size_t eflags;
  size_t eax; // More saved state (registers)
  size_t ecx;
  size_t edx;
  size_t ebx;
  size_t *esp;
  size_t *ebp;
};

void setesp(struct taskstate *u, size_t *ptr)
{
  if (ptr == 0)
    return;
  u->esp = ptr;
}

void getebp(struct taskstate *u)
{
  u->ebp = (size_t)getframe();
}

#ifdef _X86_
void tssreadax(struct taskstate *u)
{
  asm volatile("movl %%eax, %1"
               : "=r"(u->eax)
               : "r"(u->eax));
}
#endif

struct cpu
{
  int cid;
  int ncli;
  int intena;
  uint8_t apicid;
  volatile size_t started;
  struct proc *proc;
  struct context *scheduler;
};

struct cpu cpus[32]; // max 32 cores - to be changed after no. cores goes brrr
struct cpu* mycpu(void);

struct spinlock
{
  size_t locked;
  char *name;
  struct cpu *cpu;
  size_t pcs[10];
};

void initlock(struct spinlock *u, char *name)
{
  u->locked = 0;
  u->cpu = mycpu();
  u->name = name;
}

#ifdef _X86_
inline size_t xchg(volatile size_t *addr, size_t newval)
{
  size_t result;
  asm volatile("lock; xchgl %0, %1"
               : "+m"(*addr), "=a"(result)
               : "1"(newval)
               : "cc");
  return result;
}
#endif

void acquire(struct spinlock *u)
{
  CLI();
  if(u->locked == 1) return;
  while (xchg(&u->locked, 1) != 0);
}

void release(struct spinlock *u)
{
  u->locked = 0;
  u->cpu = NULL_PTR;
}

void pushcli(struct cpu *_cpu)
{
  CLI();
  _cpu->ncli++;
}

void popcli(struct cpu *_cpu)
{
  STI();
  if (_cpu->ncli > 0)
    _cpu->ncli--;
}

struct resinfo 
{
  int type; // like RES_FILE, RES_ADDR, RES_IMAGE etc.
  void* handle;
  void* used_by; // process that uses this
};

struct reslock
{
  struct spinlock lock;
  struct resinfo info;
};

void resinit(struct reslock* res, struct resinfo info)
{
  res->info = info;
  initlock(&(res->lock), NULL_PTR);
}

void resacq(struct reslock* lock)
{
  if(lock->info.handle == NULL_PTR) return; // if no resource then it 
  acquire(&(lock->lock));
}

void resrel(struct reslock* lock)
{
  if(lock->info.handle == NULL_PTR) return; // if no resource then it 
  release(&(lock->lock));
}

char *BUFFER = (char *)0xB8000;
int csr_x = 0, csr_y = 0, attrib = 0x0F;

int isvisible(int _Char)
{
  if (_Char < 32)
    return 0;
  else
    return 1;
}

static inline int strcon(char *str, char ch)
{
  while (*str != 0 && *str != 0xdeadbeef)
  {
    if (*str == ch)
      return 0;
    str++;
  }
  return 1;
}

static inline int strchr(char *str, char ch)
{
  int i = 0;
  while (str[i] != 0)
  {
    if (str[i] == ch)
      return i;
    str++;
  }
  return -1;
}

static inline char* substr(char* x, size_t start, size_t size)
{
  if(x == NULL_PTR) return (char*)NULL_PTR;
  char* p = kalloc(size + 1, 2);
  memcpy(p, x + start, size);
  p[size - 1] = '\0';
  return p;
}

// a simplistic version of strtok
static char* __strtok_last = NULL_PTR; 
static inline char* strtok0(char* str, char sep)
{
  if(str == NULL_PTR) str = __strtok_last;
  if(!__strtok_last && !str)
  {
    __strtok_last = (char*)NULL_PTR; // resets the pointer, if you use strtok for n different strings
    return (char*)NULL_PTR;
  }
  int p = strchr(str, sep);
  __strtok_last = str + p + 1;
  return substr(str, 0, p);
}

static inline void outportb(uint16_t port, uint8_t data)
{
  asm("outb %1, %0"
      :
      : "dN"(port), "a"(data));
}

static inline char *itoa(size_t x, char *s, size_t sz)
{
  if (sz < 20)
  {
    return (char *)0;
  }

  size_t tmp;
  int i, j;

  tmp = x;
  i = 0;

  do
  {
    tmp = x % 10;
    s[i++] = (tmp < 10) ? (tmp + '0') : (tmp + 'a' - 10);
  } while (x = x / 10);
  s[i--] = 0;

  for (j = 0; j < i; j++, i--)
  {
    tmp = s[j];
    s[j] = s[i];
    s[i] = tmp;
  }

  return s;
}

void clr()
{
  char *u = (char *)0xB8000;
  int i = 0;
  while (i < 80 * 25 * 2)
    u[i++] = 0;
}

void move_csr(void)
{
  unsigned temp;
  temp = csr_y * 80 + csr_x;

  outportb(0x3D4, 14);
  outportb(0x3D5, temp >> 8);
  outportb(0x3D4, 15);
  outportb(0x3D5, temp);
}

void dis_csr(void)
{
  outportb(0x3D4, 0x0A);
  outportb(0x3D5, 0x20);
}

void movecr(int x, int y)
{
  csr_x = x;
  csr_y = y;
  move_csr();
}

void scroll(void)
{
  unsigned blank, temp;

  blank = 0x20 | (attrib << 8);
  if (csr_y >= 25)
  {
    temp = csr_y - 25 + 1;
    memcpy(BUFFER, BUFFER + temp * 80, (25 - temp) * 80 * 2);
    memset(BUFFER + (25 - temp) * 80, blank, 80);
    csr_y = 25 - 1;
  }
}

void cls()
{
  unsigned blank;
  int i;

  blank = 0x20 | (attrib << 8);
  for (i = 0; i < 25; i++)
    memset(BUFFER + i * 80, blank, 80);

  csr_x = 0;
  csr_y = 0;
  move_csr();
}

void putch(unsigned char c)
{
  unsigned short *where;
  unsigned att = attrib << 8;

  if (c == 0x08)
  {
    if (csr_x != 0)
      csr_x--;
  }

  else if (c == 0x09)
  {
    csr_x = (csr_x + 8) & ~(8 - 1);
  }

  else if (c == '\r')
  {
    csr_x = 0;
  }

  else if (c == '\n')
  {
    csr_x = 0;
    csr_y++;
  }

  else if (c >= 48 && c <= 58 || c >= 'a' && c <= 'z')
  {
    where = BUFFER + (csr_y * 80 + csr_x);
    *where = c;
    *(where + 1) = 0xF;
    csr_x++;
  }

  else if (c >= ' ')
  {
    where = BUFFER + (csr_y * 80 + csr_x);
    *where = c | att;
    *(where + 1) = 0xF;
    csr_x++;
  }

  if (csr_x >= 80)
  {
    csr_x = 0;
    csr_y++;
  }

  scroll();
  move_csr();
}

void kprintint(int n)
{
  char buf[30];
  itoa(n, buf, sizeof(buf));
  kprint(buf);
}

void kprint(char *text)
{
  while (*text)
  {
    *BUFFER++ = *text++;
    *BUFFER++ = 0xF;
  }
}

void printint(int xx, int base, int sign)
{
  char digits[] = "0123456789abcdef";
  char buf[16];
  int i;
  uint8_t x;

  if(sign && (sign = xx < 0))
    x = -xx;
  else
    x = xx;

  i = 0;
  do{
    buf[i++] = digits[x % base];
  }while((x /= base) != 0);

  if(sign)
    buf[i++] = '-';

  while(--i >= 0)
    putch(buf[i]);
}

void perror(char *text)
{
  while (*text)
  {
    *BUFFER++ = *text++;
    *BUFFER++ = 0x4;
  }
  HALT();
}

void pwarn(char *text)
{
  while (*text)
  {
    *BUFFER++ = *text++;
    *BUFFER++ = 0x2;
  }
}

// TO DO: Check actual definitions of these macros
#define va_start(arg, list) list = (va_list)&arg
#define va_arg(list, type) *(type*)(list); list += sizeof(type)
#define va_end(list) list = (va_list)NULL_PTR

void vsnprintf(char* buf, size_t sz, const char* fmt, va_list args)
{
  char m;
  while(*fmt && sz > 0)
  {
    m = *fmt; 
    fmt++;
    if(m == '%')
    {
      m = *(fmt++);
      switch(m)
      {
      case 'i':
        char* s = itoa(*(int*)args, buf, sz);
        args += sizeof(int);
        sz -= (s - buf); // decrease the size
      break;
      case 's':
        int l = min(sz, strlen(*(char**)args));
        memcpy(buf, *(char**)args, l);
        args += sizeof(char*);
        sz -= l;
      break;
      case 'p':
        
      break;
      }
    } else {
      *buf++ = *fmt;
    }
  }
}

void printf(char* code, ...)
{
  va_list list;
  char buf[300];
  va_start(code, list);
  char* fmt = va_arg(list, char*);
  vsnprintf(buf, sizeof buf, fmt, list);
  kprint(buf);
  va_end(list);
}

#define assert(x) do {  \
    if(!(x)) printf("Assert failed: %s @ %s:%i", #x, __FILE__, __LINE__); \
} while(0)

void kersc()
{
  *BUFFER = (char)0;
  BUFFER--;
}

void kputc(char t)
{
  putch(t);
}

void showint(int p)
{
  int u;
  while (p)
  {
    u = p % 10;
    char t = (u + '0') % 58;
    kputc(t);
    p = p / 10;
  }
}

uint64_t rseed = 0xFEE00000;
uint32_t rand()
{
  rseed ^= (rseed << 13);
  rseed ^= (rseed >> 7);
  rseed ^= (rseed << 17);
  return (uint32_t)rseed;
}

double randf()
{
  uint32_t r = rand();
  return (double)r / (double)(1 << 30);
}

struct regs
{
  unsigned int gs, fs, es, ds;
  unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
  unsigned int int_no, err_code;
  unsigned int eip, cs, eflags, useresp, ss;
};

static inline size_t readeflags()
{
  size_t eflags;
  asm volatile("pushfl; popl %0"
               : "=r"(eflags));
  return eflags;
}

int checkintr()
{
  size_t flags = readeflags();
  return flags & (1 << 9);
}

static inline void pusheflags(size_t flags)
{
  asm volatile (
        "push  %[flags]\n"   // Push the new EFLAGS value onto the stack
        "popf\n"                 // Pop the new value into EFLAGS
        :
        : [flags] "rm" (flags)
        : "flags"  // clobbered register: informs the compiler that EFLAGS is modified
  );
}

static void set_iopl(char x, char y)
{
  size_t i = readeflags();
  i |= (x << 12);
  i |= (y << 13);
  pusheflags(i);
}

static void no_iopl()
{
  size_t i = readeflags();
  i &= ~(1 << 12);
  i &= ~(1 << 13);
  pusheflags(i);
}

void iopriv()
{
  set_iopl(1,1);
}

void wset(wchar_t *wch, char chrs[2])
{
  if (wch != 0)
    *wch = (chrs[0] << 8) | (chrs[1]);
}

void wget(wchar_t *wch, char chrs[2])
{
}

void *memmove(void *dst, const void *src, size_t n)
{
  const char *s;
  char *d;

  s = (const char *)src;
  d = (char *)dst;
  if (s < d && s + n > d)
  {
    s += n;
    d += n;
    while (n-- > 0)
      *(--d) = *(--s);
  }
  else
  {
    while (n-- > 0)
      *(d++) = *(s++);
  }
  return dst;
}

int memncmp(char* str1, char* str2, size_t len)
{
  int sz = 0;
  while (*str1 != 0 && *str2 != 0 && sz < len)
  {
    if (*str1 > *str2) {
      return 1;
    } else if (*str1 < *str2) {
      return -1;
    } else;
    str1++, str2++, sz++; // i was dumb
  }
  return 0;
}

int memcmp(char *str1, char *str2)
{
  int is_eq = 1;
  while (*str1 != 0 && *str2 != 0)
  {
    if (*str1 > *str2)
    {
      return 1;
    }
    if (*str1 < *str2)
    {
      return -1;
    }
    if (*str1 == *str2)
    {
      is_eq = 1;
    }
    str1++, str2++; // i was dumb
  }
  return 0;
}

#define ERR_INVSTR -256 // invalid string error

int strcmp(char *str1, char *str2)
{
  if (str1 == 0 || str2 == 0)
    return ERR_INVSTR;
  return memcmp(str1, str2);
}

struct unistr
{ // Unicode String
  wchar_t *chars;
  size_t size;
};

void strrep(char *str, char chr1, char chr2)
{
  int i = 0;
  while (str[i] != 0)
  {
    if (str[i] == chr1)
      str[i] = chr2;

    i++;
  }
}

void strrep2(char *str, char chr1)
{
  strrep(str, chr1, (char)0);
}

wchar_t readustr(struct unistr i, size_t off)
{
  if (off < i.size)
    return *(i.chars + off);
  else
    return (wchar_t)0;
}

void raise(int code)
{
  errno = code;
  HALT();
}

void geteflags(struct taskstate *t)
{
  t->eflags = readeflags();
}

uint64_t mminq(void *addr)
{
  uint64_t ret;
  asm volatile("mov %1, %0"
               : "=r"(ret)
               : "m"((uint64_t *)addr)
               : "memory");
  return ret;
}

void mmoutq(void *addr, uint64_t data)
{
  uint64_t ret;
  asm volatile("mov %0, %1"
               : "=r"(ret)
               : "m"((uint64_t *)addr)
               : "memory");
  return ret;
}

void snprintf(char* x, char* y, void* args)
{
    int a = 0, argc = 0;
    while (x[a] && y[a])
    {
        if (y[a] != '%' && y[a + 1] != 'i')
            x[a] = y[a];
        if (y[a] == '%' && y[a + 1] == 's')
        {
            char* n = ((char**)args)[argc];
            memcpy(y + a + 1, n, strlen(n));
            argc++;
        }
        a++;
    }

}

char *strstr(const char *s1, const char *s2)
{
  size_t n = strlen(s2);
  while (*s1)
    if (!memcmp(s1++, s2))
      return (char *)(s1 - 1);
  return 0;
}

void strcpy(char* dest, const char* src)
{
  int len = min(strlen(dest), strlen(src));
  memcpy(dest, src, len);
}