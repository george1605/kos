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
size_t xhciBase = pci_baseaddr(0);

typedef struct
{
  uint8_t capLength; // Capability Register Length
  uint8_t reserved;
  uint16_t hciVersion; // Interface Version Number
  uint32_t hcsParams1;
  uint32_t hcsParams2;
  uint32_t hcsParams3;
  union
  {
    uint32_t hccParams1;
    struct
    {
      uint32_t addressCap64 : 1;     // 64-bit Addressing Capability
      uint32_t bwNegotationCap : 1;  // BW Negotiation Capability
      uint32_t contextSize : 1;      // Context size (1 = 64 bytes, 0 = 32 byte)
      uint32_t portPowerControl : 1; // Port power control capability
      uint32_t portIndicators : 1;   // Port indicator control capability
      uint32_t lightHCResetCap : 1;  // Capable of light host controller reset
      uint32_t latencyToleranceMessagingCapability : 1;
      uint32_t noSecondarySIDSupport : 1;
      uint32_t parseAllEventData : 1;
      uint32_t shortPacketCapability : 1;
      uint32_t stoppedEDTLACap : 1;
      uint32_t contiguousFrameIDCap : 1;
      uint32_t maxPSASize : 4;
      uint32_t extendedCapPointer : 16; // XHCI extended capabilities pointer
    } __attribute__((packed));
  };
  uint32_t dbOff;  // Doorbell offset
  uint32_t rtsOff; // Runtime registers space offset
  uint32_t hccParams2;
} __attribute__((packed)) xhci_regs;

  struct _xhcicon
  {
    int *contr;
    size_t len;
    size_t size;
  } xhcilist;

  int xhcinew()
  {
    enable_mastering(0, 0, 0);
    enable_interrupt(0, 0, 0);
    size_t xhciVirtual = (size_t)iomapping((long)xhciBase);
    xhci_regs* reg0 = (xhci_regs*)xhciVirtual;
  }