#pragma once
#include "../mem.h"
#include "../pci.h"
#include "hci.h"
#define USB_HOST 0xE000
#define USB_HOSTMAX 0xE01F
#define USB_DRIVNO 20

#define US_URB_ACTIVE	0	
#define US_SG_ACTIVE	1	
#define US_ABORTING	2	
#define US_DISCONNECTING	3	
#define US_RESETTING	4	
#define US_TIMED_OUT	5	
#define US_SCAN_PENDING	6	
#define US_REDO_READ10	7	
#define US_READ10_WORKED	8	

char USB_DRIVNAME[2] = {'d', USB_DRIVNO + 'C'};
typedef enum {
  usbstick,
  usbmouse,
  usbcam,
  usbfront 
} usbtype;

struct usbdev {
  struct spinlock lock;
  void* context;
  int events[32];
};

struct usbdriv {
  char* name;
  void(*start)(void);
  void(*startio)(void);
  struct usbdev* dev;
};

void lockusb(struct usbdev u){
  acquire(&u.lock);
}

void unlockusb(struct usbdev u){
  release(&u.lock);
}

int testusb(){
   return (inb(USB_HOST) & 0x20);
}

void usbcheck(){}

void usbfree(struct usbdev* u){
  free(u->context);
  u->events[0] = 0;
}
    
void usbattach(struct usbdev u,struct usbdriv* k){
  k->dev = &u;
}

uint8_t xhciClassCode = PCI_CLASS_SERIAL_BUS;
uint8_t xhciSubclass = PCI_SUBCLASS_USB;
uint8_t xhciProgIF = PCI_PROGIF_XHCI;

struct _xhcicon {
  int* contr;
  size_t len;
  size_t size;
} xhcilist;
 
int xhcinew(){
  
}