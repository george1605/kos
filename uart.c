#pragma once
#include "irq.c"
#define COM1 0x3f8
#define COM2 0x2f8
#define COM3 0x3e8
#define IRQ_COM1 4
static int uart;

struct srldev {
  void* buffer;
  int com; // the com number
  struct spinlock lock;
};

void srlinit(struct srldev u){
  u.com = COM1;
  initlock(&u.lock,"serial device");
}

void uartputc(int c){
  int i;

  if(!uart)
    return;
  for(i = 0; i < 128 && !(inb(COM1+5) & 0x20); i++);
  outb(COM1 + 0, c);
}

int uartgetc(void){
  if(!uart)
    return (-1);
  if(!(inb(COM1 + 5) & 0x01))
    return (-1);

  return inb(COM1 + 0);
}

void uartputs(char* ptr){
   char* p;
   if(ptr == 0) return;

   for(p = ptr; *p != 0; p++)
     uartputc(*p);
}

void uartinit(void){
  char *p;

  outb(COM1 + 2, 0);

  outb(COM1 + 3, 0x80);
  outb(COM1 + 0, 115200/9600);
  outb(COM1 + 1, 0);
  outb(COM1 + 3, 0x03);
  outb(COM1 + 4, 0);
  outb(COM1 + 1, 0x01);

  if(inb(COM1 + 5) == 0xFF)
    return;
  uart = 1;

  inb(COM1 + 2);
  inb(COM1 + 0);

  for(p="KOS...\n"; *p; p++)
    uartputc(*p);
}

void uartnewl(){
  uartputc('\n');
}

static int serial_init() {
   outb(COM1 + 1, 0x00);    // Disable all interrupts
   outb(COM1 + 3, 0x80);    // Enable DLAB (set baud rate divisor)
   outb(COM1 + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
   outb(COM1 + 1, 0x00);
   outb(COM1 + 3, 0x03);    // 8 bits, no parity, one stop bit
   outb(COM1 + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
   outb(COM1 + 4, 0x0B);    // IRQs enabled, RTS/DSR set
   outb(COM1 + 4, 0x1E);    // Set in loopback mode, test the serial chip
   outb(COM1 + 0, 0xAE);    // Test serial chip (send byte 0xAE and check if serial returns same byte)

   if(inb(COM1 + 0) != 0xAE) {
      return 1;
   }

   outb(COM1 + 4, 0x0F);
   return 0;
}

void serial_handler(struct regs *r) {
	char serial = (char)uartgetc();
	irq_ack(IRQ_COM1);
	switch (serial) {
		case 127:
			serial = 0x08;
			break;
		case 13:
			serial = '\n';
			break;
		default:
			break;
	}
}

void serial_install(){
  irq_install_handler(IRQ_COM1, serial_handler);
}