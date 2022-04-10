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

void keyboard_wait()
{
  while (inportb(0x60) & 0x20 != 0)
    ;
}

int getch()
{
  keyboard_wait();
  return kbdbuf[kbdindex];
}

char *gets(size_t chars)
{
  char *i = alloc(0, chars);
  int j;
  for (j = 0; j < chars; j++)
    i[j] = getch();
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