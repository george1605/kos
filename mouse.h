#include "port.h"
#include "irq.c"
#include "vfs.h"
#define MOUSE_LEFT_BUTTON(x) x
#define MOUSE_RIGHT_BUTTON(x) x

#define MOUSE_PORT   0x60
#define MOUSE_STATUS 0x64
#define MOUSE_ABIT   0x02
#define MOUSE_BBIT   0x01
#define MOUSE_WRITE  0xD4
#define MOUSE_F_BIT  0x20
#define MOUSE_V_BIT  0x08
#define MOUSE_IRQ 12
struct mouse {
  int status;
  int key;
  int buffer[3];
  struct spinlock lock;
} cmouse;

int mouse_x;
int mouse_y;

void mouse_wait(uint8_t a_type) {
	size_t timeout = 100000;
	if (!a_type) {
		while (--timeout) {
			if ((inportb(MOUSE_STATUS) & MOUSE_BBIT) == 1) {
				return;
			}
		}
		return;
	} else {
		while (--timeout) {
			if (!((inportb(MOUSE_STATUS) & MOUSE_ABIT))) {
				return;
			}
		}
		return;
	}
}

void mouse_write(uint8_t write) {
	mouse_wait(1);
	outportb(MOUSE_STATUS, MOUSE_WRITE);
	mouse_wait(1);
	outportb(MOUSE_PORT, write);
}

uint8_t mouse_read() {
	mouse_wait(0);
	char t = inportb(MOUSE_PORT);
	return t;
}

extern void desktop_trigger_mouse(int x, int y);
void mouse_handler(struct regs* r){
	size_t mouse_cycle = 0;
	size_t mouse_bytes[3];

	switch (mouse_cycle)
	{
	case 0: 
		mouse_bytes[0] = mouse_read();
		if (mouse_bytes[0])
		{
			cmouse.buffer[0] = 1;
		}
		else
		{
			cmouse.buffer[0] = 0;
		}

		if (mouse_bytes[0])
		{
			cmouse.buffer[2] = 1;
		}
		else
		{
			cmouse.buffer[2] = 0;
		}
		mouse_cycle++;
		break;
	case 1:
		mouse_bytes[1] = mouse_read();
		mouse_cycle++;
		break;
	case 2:
		mouse_bytes[2] = mouse_read();
		mouse_x = mouse_x + (mouse_bytes[1]);
		mouse_y = mouse_y - (mouse_bytes[2]);

		// Adjust mouse position
		if (mouse_x < 0)
			mouse_x = 0;
		if (mouse_y < 0)
			mouse_y = 0;
	}
	desktop_trigger_mouse(mouse_x, mouse_y);
}

void mouse_install() {
	uint8_t status;
	CLI();
	mouse_wait(1);
	outportb(MOUSE_STATUS, 0xA8);
	mouse_wait(1);
	outportb(MOUSE_STATUS, 0x20);
	mouse_wait(0);
	status = inportb(0x60) | 2;
	mouse_wait(1);
	outportb(MOUSE_STATUS, 0x60);
	mouse_wait(1);
	outportb(MOUSE_PORT, status);
	mouse_write(0xF6);
	mouse_read();
	mouse_write(0xF4);
	mouse_read();
	STI();
	irq_install_handler(MOUSE_IRQ, mouse_handler);
}

void mouse_init() {
	mouse_install();
}

struct vfile mouse_map(){
  struct blkdev u;
  u.portno = MOUSE_PORT;
  u.name = "MOUSE";
  initlock(&u.lock, u.name); 
  sysvf[9] = vfsdmap("/home/dev/mouse.dev",u);
  return sysvf[9];
}

void mouse_lock(){
  acquire(&cmouse.lock);
}

void mouse_unlock(){
  release(&cmouse.lock);
}