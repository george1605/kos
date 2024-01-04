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

  struct xhci_cap_regs {
	volatile uint32_t cap_caplen_version;
	volatile uint32_t cap_hcsparams1;
	volatile uint32_t cap_hcsparams2;
	volatile uint32_t cap_hcsparams3;
	volatile uint32_t cap_hccparams1;
	volatile uint32_t cap_dboff;
	volatile uint32_t cap_rtsoff;
	volatile uint32_t cap_hccparams2;
} __attribute__((packed));

struct xhci_port_regs {
	volatile uint32_t port_status;
	volatile uint32_t port_pm_status;
	volatile uint32_t port_link_info;
	volatile uint32_t port_lpm_control;
} __attribute__((packed));

struct xhci_op_regs {
	volatile uint32_t op_usbcmd;   /* 0 */
	volatile uint32_t op_usbsts;   // 4
	volatile uint32_t op_pagesize; // 8h
	volatile uint32_t op__pad1[2]; // ch 10h
	volatile uint32_t op_dnctrl;   // 14h
	volatile uint32_t op_crcr[2];     // 18h 1ch
	volatile uint32_t op__pad2[4]; // 20h 24h 28h 2ch
	volatile uint32_t op_dcbaap[2];   // 30h 34h
	volatile uint32_t op_config;   // 38h
	volatile uint8_t  op_more_padding[964]; // 3ch-400h
	struct xhci_port_regs op_portregs[256];
} __attribute__((packed));

struct xhci_trb {
	uint32_t trb_thing_a;
	uint32_t trb_thing_b;
	uint32_t trb_status;
	uint32_t trb_control;
} __attribute__((packed));

struct xhcictrlinfo {
	void* mmio;
	uint32_t device;
	uint64_t pcie_offset;
	struct xhci_cap_regs * cregs;
	struct xhci_op_regs * oregs;
	struct proc * thread;
	volatile struct xhci_trb * cr_trbs;
	volatile struct xhci_trb * er_trbs;
	spinlock command_queue;
	uint32_t command_queue_cycle;
	int command_queue_enq;
	volatile uint32_t * doorbells;
};

static int xhci_command(struct xhcictrlinfo * controller, uint32_t p1, uint32_t p2, uint32_t status, uint32_t control) 
{
	acquire(&(controller->command_queue));
	control &= ~1;
	control |= controller->command_queue_cycle;

	controller->cr_trbs[controller->command_queue_enq].trb_thing_a = p1;
	controller->cr_trbs[controller->command_queue_enq].trb_thing_b = p2;
	controller->cr_trbs[controller->command_queue_enq].trb_status  = status;
	controller->cr_trbs[controller->command_queue_enq].trb_control = control;

	controller->command_queue_enq++;
	if (controller->command_queue_enq == 63) {
		controller->cr_trbs[controller->command_queue_enq].trb_control ^= 1;
		if (controller->cr_trbs[controller->command_queue_enq].trb_control & (1 << 1)) {
			controller->command_queue_cycle ^= 1;
		}
		controller->command_queue_enq = 0;
	}

	/* ring doorbell */
	controller->doorbells[0] = 0;

	release(&(controller->command_queue));
	return 0;
}

size_t xhci_write(struct vfile* node, size_t offset, size_t size, uint8_t * buffer) {
	struct xhcictrlinfo * controller = (struct xhcictrlinfo*)node->mem;
	if (size != sizeof(struct xhci_trb)) return -1;
	struct xhci_trb * data = (struct xhci_trb*)buffer;
	xhci_command(controller, data->trb_thing_a, data->trb_thing_b, data->trb_status, data->trb_control);
	return sizeof(struct xhci_trb);
}

uint64_t find_xhci(uint32_t device, uint16_t v, uint16_t d, void * extra)
{
  if (pci_find_type(device) != 0x0C03) return;
  struct pcidev _d = {.device = device};
	if (pciget(_d, PCI_PROG_IF) != 0x30) return;
	struct vfile* vf = (struct vfile*)extra;

	uint16_t command_reg = pciget(_d, PCI_COMMAND);
	command_reg |= (1 << 2);
	command_reg |= (1 << 1);
	pciset(_d, PCI_COMMAND, command_reg);

	/* The mmio address is 64 bits and combines BAR0 and BAR1... */
	uint64_t addr_low  = pciget(_d, PCI_BAR0) & 0xFFFFFFF0;
	uint64_t addr_high = pciget(_d, PCI_BAR1) & 0xFFFFFFFF; /* I think this is right? */
	uint64_t mmio_addr = (addr_high << 32) | addr_low;

	if (mmio_addr == 0) {
		/* Need to map... */
		return;
		#if 0
		mmio_addr = mmu_allocate_n_frames(2) << 12;
		pci_write_field(device, PCI_BAR0, 4, (mmio_addr & 0xFFFFFFF0) | (1 << 2));
		pci_write_field(device, PCI_BAR1, 4, (mmio_addr >> 32));
		#endif
	}

	return mmio_addr;
}

  int xhcinew()
  {
    enable_mastering(0, 0, 0);
    enable_interrupt(0, 0, 0);
    size_t xhciVirtual = (size_t)iomapping((long)xhciBase);
    xhci_regs* reg0 = (xhci_regs*)xhciVirtual;
  }