#pragma once
#include "lib.c"
#include "system.h"
#include "irq.c"
#define SCROLL_LED 1
#define NUM_LED 2
#define CAPS_LED 4
#define CTL(x) (x - '@')
#define IS_PRESSED(buf) !((buf >> 7) & 1)

extern int glsig;
int kbignore = 0;
typedef char *kblayout;

unsigned char kbdus[128] =
    {
        0,
        27,
        '1',
        '2',
        '3',
        '4',
        '5',
        '6',
        '7',
        '8', /* 9 */
        '9',
        '0',
        '-',
        '=',
        '\b',
        '\t',
        'q',
        'w',
        'e',
        'r',
        't',
        'y',
        'u',
        'i',
        'o',
        'p',
        '[',
        ']',
        '\n',
        0,
        'a',
        's',
        'd',
        'f',
        'g',
        'h',
        'j',
        'k',
        'l',
        ';',
        '\'',
        '`',
        0,
        '\\',
        'z',
        'x',
        'c',
        'v',
        'b',
        'n',
        'm',
        ',',
        '.',
        '/',
        0,
        '*',
        0,
        ' ',
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        '-',
        0,
        0,
        0,
        '+',
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
};

void getc_stdin()
{
  int code = inportb(0x60);
  struct proc* p = myproc();
  char* s = (char*)p->std[0].mem;
  s[0] = code;
}

void keyboard_wait()
{
  while (inportb(0x60) & 0x20 != 0)
    ;
}

void gets_stdin(int until)
{
  int code = inportb(0x60);
  int c = 0;
  struct proc* p = myproc();
  while(code != until && c < p->std[0].size)
  {
    keyboard_wait();
    code = inportb(0x60);
    p->std[0].mem[c] = code;
    c++;
  }
  p->std[0].mem[c] = '\0';
}

char *kbdbuf = (char *)(0x2C00FF);
uint16_t kbdindex = 0;
void keyboard_handler(struct regs *r)
{
  unsigned char scancode;
  scancode = inportb(0x60);

  if (scancode & 0x80 || kbignore)
  {
  }
  else
  {
    if (scancode == 27)
      glsig = 0x20;

    if (kbdindex == 0)
      kbdindex = 0x80;

    kbdbuf[kbdindex++] = kbdus[scancode];
  }
}

int getch()
{
  keyboard_wait();
  return kbdbuf[kbdindex];
}

char *gets(size_t chars)
{
  char *i = (char*)kalloc(chars, KERN_MEM);
  int j;
  for (j = 0; j < chars; j++)
    i[j] = getch();
  return i;
}

char* getl()
{
  char* i = (char*)kalloc(64, KERN_MEM);
  int k = 0;
  while(k < 64)
  { 
    i[k] = getch();
    if(i[k] == '\n') break;
    k++;
  }
  return i;
}

void keyboard_install()
{
  irq_install_handler(1, keyboard_handler);
}

void kbd_init()
{
  keyboard_install();
}

size_t keyboard_restart()
{
  int data = inportb(0x61);
  outportb(0x61, data | 0x80);
  outportb(0x61, data & 0x7F);
  return 0;
}

void keyboard_setled(int ledno)
{
  if (ledno > 5)
    return;

  while ((inportb(0x64) & 2) != 0)
    ;
  outportb(0x60, 0xED);
  while ((inportb(0x64) & 2) != 0)
    ;
  outportb(0x60, ledno);
}