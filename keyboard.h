#include "lib.c"
#include "isr.c"
#include "irq.c"

#define KEY_PORT 0x60
void keyboard_handler(struct regs *r)
{
    unsigned char scancode;
    scancode = inportb(0x60);
    if (scancode & 0x80)
    {

    }
    else
    {
      if(scancode == 27 || scancode == 112)
        return;

      if(scancode == 8)
        kersc();

      kputc(scancode);
    }
}

void keyboard_install(){
  irq_install_handler(1,keyboard_handler);
}

void kbd_init(){
  keyboard_install();
}
