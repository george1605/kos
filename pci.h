#pragma once
#include "lib.c"
#include "mem.h"
#include "port.h"
#include "vfs.h"
#define AMDGPU 0x1022
#define INTELD 0x8086

#define PCI_PROG_IF  0x09 // 1
#define PCI_SUBCLASS 0x0a // 1
#define PCI_CLASS    0x0b // 1
#define PCI_CACHE_LINE_SIZE 0x0c // 1
#define PCI_LATENCY_TIMER   0x0d // 1
#define PCI_HEADER_TYPE     0x0e // 1
#define PCI_BIST            0x0f // 1
#define PCI_BAR0            0x10 // 4
#define PCI_BAR1            0x14 // 4
#define PCI_BAR2            0x18 // 4
#define PCI_BAR3            0x1C // 4
#define PCI_BAR4            0x20 // 4
#define PCI_BAR5            0x24 // 4

#define PCI_ADDR 0xCF8
#define PCI_DATA 0xCFC
#define FL_BASE_MASK 0x0007
#define FL_BASE0 0x0000
#define FL_BASE1 0x0001
#define FL_BASE2 0x0002
#define FL_BASE3 0x0003
#define FL_BASE4 0x0004
#define FL_GET_BASE(x) (x & FL_BASE_MASK)

#define PCI_VENDOR_ID            0x00 // 2
#define PCI_DEVICE_ID            0x02 // 2
#define PCI_COMMAND              0x04 // 2
#define PCI_STATUS               0x06 // 2
#define PCI_REVISION_ID          0x08 // 1

#define PCI_CLASS_UNCLASSIFIED 0x0
#define PCI_CLASS_STORAGE 0x1
#define PCI_CLASS_NETWORK 0x2
#define PCI_CLASS_DISPLAY 0x3
#define PCI_CLASS_MULTIMEDIA 0x4
#define PCI_CLASS_MEMORY 0x5
#define PCI_CLASS_BRIDGE 0x6
#define PCI_CLASS_COMMUNICATION 0x7
#define PCI_CLASS_PERIPHERAL 0x8
#define PCI_CLASS_INPUT_DEVICE 0x9
#define PCI_CLASS_DOCKING_STATION 0xA
#define PCI_CLASS_PROCESSOR 0xB
#define PCI_CLASS_SERIAL_BUS 0xC // USB, here I come!!!
#define PCI_CLASS_WIRELESS_CONTROLLER 0xD
#define PCI_CLASS_INTELLIGENT_CONTROLLER 0xE
#define PCI_CLASS_SATELLITE_COMMUNICATION 0xF
#define PCI_CLASS_ENCRYPTON 0x10
#define PCI_CLASS_SIGNAL_PROCESSING 0x11
#define PCI_CLASS_COPROCESSOR 0x40
#define PCI_PROGIF_XHCI 0x30
#define PCI_INTERRUPT_LINE 0x3c

#define PCI_SUBCLASS_IDE 0x1
#define PCI_SUBCLASS_FLOPPY 0x2
#define PCI_SUBCLASS_ATA 0x5
#define PCI_SUBCLASS_SATA 0x6
#define PCI_SUBCLASS_NVM 0x8
#define PCI_SUBCLASS_ETHERNET 0x0
#define PCI_SUBCLASS_USB 0x3

#define PCI_CMD_INTERRUPT_DISABLE (1 << 10)
#define PCI_CMD_SPECIAL_CYCLES (1 << 3)
#define PCI_CMD_BUS_MASTER (1 << 2)
#define PCI_CMD_MEMORY_SPACE (1 << 1)
#define PCI_CMD_IO_SPACE (1 << 0)

struct pcidev
{
	size_t vendor;
	size_t device;
	size_t func;
	size_t pclass;
	size_t subclass;
	size_t progif;

	size_t bus;
	size_t slot;
	size_t bits;
	void (*driver)(struct pcidev *_This);
} * pci_devs, ata_device;

size_t pcinum = 0;
size_t pcimap[100];

struct pcinfo
{
	uint16_t bus;
	uint16_t device;
	uint16_t function;
	uint16_t pclass;
	uint16_t subclass;
};

size_t pciget(struct pcidev dev, size_t field)
{
	outportl(0xCF8, dev.bits);

	uint32_t size = pcimap[field];
	if (size == 1)
	{
		uint8_t t = inportb(0xCFC + (field & 3));
		return t;
	}
	else if (size == 2)
	{
		uint16_t t = inports(0xCFC + (field & 2));
		return t;
	}
	else if (size == 4)
	{
		size_t t = inportl(0xCFC);
		return t;
	}
	return 0xffff;
}

static inline int pci_extract_bus(uint32_t device) {
	return (uint8_t)((device >> 16));
}
static inline int pci_extract_slot(uint32_t device) {
	return (uint8_t)((device >> 8));
}
static inline int pci_extract_func(uint32_t device) {
	return (uint8_t)(device);
}

uint32_t* pcie_addr(uint32_t device, int field) {
	return (uint32_t*)((pci_extract_bus(device) << 20) | (pci_extract_slot(device) << 15) | (pci_extract_func(device) << 12) | (field));
}

struct vfile* pcie_map(uint32_t device, int field) {
	struct vfile* vf = (struct vfile*)kalloc(sizeof(struct vfile), KERN_MEM);
	vf->mem = pcie_addr(device, field);
	vf->drno = device;
	return vf;
}

void pciset(struct pcidev dev, size_t field, size_t value)
{
	outportl(0xCF8, dev.bits);
	outportl(0xCFC, value);
}

uint16_t pciread(uint8_t bus, uint8_t slot, uint8_t func, uint8_t off)
{
	size_t address;
	size_t lbus = (size_t)bus;
	size_t lslot = (size_t)slot;
	size_t lfunc = (size_t)func;
	uint16_t tmp = 0;

	address = (size_t)((lbus << 16) | (lslot << 11) |
					   (lfunc << 8) | (off & 0xfc) | ((size_t)0x80000000));

	outportl(0xCF8, address);
	tmp = (uint16_t)((inportl(0xCFC) >> ((off & 2) * 8)) & 0xffff);
	return tmp;
}

uint16_t pci_readt(struct pcidev dev, int off)
{
	return pciread(dev.bus, dev.slot, dev.func, off);
}

void pciwrite(size_t address)
{
	asm("outl %1, %0" ::"dN"((uint16_t)(PCI_ADDR)), "a"(address)); // maybe outportl(PCI_ADDR,address) ?
}

uint16_t pciread_vendor(uint16_t bus, uint16_t device, uint16_t function)
{
	size_t r0 = pciread(bus, device, function, 0);
	return r0;
}

uint16_t pciread_device(uint16_t bus, uint16_t device, uint16_t function)
{
	size_t r0 = pciread(bus, device, function, 2);
	return r0;
}

uint16_t pciread_progif(uint16_t bus, uint16_t device, uint16_t function)
{
	size_t r0 = pciread(bus, device, function, 0x9);
	return r0;
}

uint16_t pciread_class(uint16_t bus, uint16_t device, uint16_t function)
{
	size_t r0 = pciread(bus, device, function, 0xB);
	return r0;
}

uint16_t pciread_sclass(uint16_t bus, uint16_t device, uint16_t function)
{
	size_t r0 = pciread(bus, device, function, 0xA);
	return r0;
}

uint16_t pciread_head(uint16_t bus, uint16_t device, uint16_t function)
{
	size_t r0 = pciread(bus, device, function, 0xE);
	return r0;
}

void pci_fill(struct pcidev *_Pci, struct pcinfo _Info)
{
	_Pci->device = pciread_device(_Info.bus, _Info.device, _Info.function);
	_Pci->pclass = pciread_class(_Info.bus, _Info.device, _Info.function);
	_Pci->subclass = pciread_sclass(_Info.bus, _Info.device, _Info.function);
	return;
}

int pci_check(struct pcinfo u)
{
	int k = pciread_vendor(u.bus, u.device, u.function);
	if (k == 0xffff)
		return 1;
	else
		return 0;
}

int pci_checks(int bus, int dev, int func)
{
	int k = pciread_vendor(bus, dev, func);
	if (k == 0xffff)
		return 1;
	else
		return 0;
}

int pci_add(int bus, int slot, int func)
{
	if (pcinum == 31)
		return 0;
	struct pcidev device;

	device.vendor = pciread_device(bus, slot, func);
	device.device = pciread_device(bus, slot, func);

	device.bus = bus;
	device.slot = slot;
	device.func = func;

	device.pclass = pciread_class(bus, slot, func);
	device.subclass = pciread_sclass(bus, slot, func);
	device.progif = pciread_progif(bus, slot, func);

	pci_devs[++pcinum] = device;
	return pcinum;
}

void pci_setup(struct pcidev *_Devs, int _DevNo)
{
	struct pcidev d;
	for (uint16_t i = 0; i < 256; i++)
	{ // Bus
		for (uint16_t j = 0; j < 32; j++)
		{ // Slot
			if (pci_checks(i, j, 0))
			{
				int index = pci_add(i, j, 0);
				d = _Devs[index];
				if (pciread_head(d.bus, d.slot, d.func) & 0x80)
				{
					for (int k = 1; k < 8; k++)
					{ // Func
						if (pci_checks(i, j, k))
						{
							pci_add(i, j, k);
							printf("Found PCI device [%i, %i, %i]\n", i, j, k);
						}
					}
				}
			}
		}
	}
}

void pci_init()
{
	pci_devs = (struct pcidev *)alloc(0, 32 * sizeof(struct pcidev));
	pci_setup(pci_devs, 31);
}

void pci_unload()
{
	if (pci_devs != 0)
		free(pci_devs); // frees the memory
}

struct pcidev pci_get_device(int vendor, int device, int bus)
{
	int a;
	for (a = 0; a < 31; a++)
		if (pci_devs[a].vendor == vendor && pci_devs[a].device == device)
			return pci_devs[a];
}

struct pcidev get_audiodev()
{
	int a;
	for (a = 0; a < 31; a++)
		if (pci_devs[a].pclass == 0x4 && pci_devs[a].subclass == 0x3)
			return pci_devs[a];
}

void pciwrites(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint16_t data)
{
	uint32_t address = (uint32_t)((bus << 16) | (slot << 11) | (func << 8) | (offset & 0xfc) | 0x80000000);

	outportl(0xCF8, address);
	outportl(0xCFC, (inportl(0xCFC) & (~(0xFFFF << ((offset & 2) * 8)))) |
						(uint32_t)(data << ((offset & 2) * 8)));
}

void enable_mastering(int bus, int slot, int funcs) 
{ 
	pciwrites(bus, slot, funcs, 0x4, pciread(bus, slot, funcs, 0x4) | PCI_CMD_BUS_MASTER);
}

void enable_interrupt(int bus, int slot, int funcs)
{ 
	pciwrites(bus, slot, funcs, 0x4, pciread(bus, slot, funcs, 0x4) & (~PCI_CMD_INTERRUPT_DISABLE)); 
}

inline size_t pci_baseaddr(uint8_t idx, struct pcidev dev)
{
	if(idx < 0 || idx > 5) return -1; 

	size_t bar = pciread(dev.bus, dev.slot, dev.func, 0x10 + (idx * sizeof(uint32_t)));
	if (!(bar & 0x1)  && bar & 0x4  && idx < 5)
	{
		bar |= ((size_t)(pciwrites(dev.bus, dev.slot, dev.func, 0x10 + ((bar + 1) * sizeof(uint32_t)), 0)) << 32); // what is happening here? must fix
	}

	return (bar & 0x1) ? (bar & 0xFFFFFFFFFFFFFFFC) : (bar & 0xFFFFFFFFFFFFFFF0);
}

uint32_t pci_read_field(uint32_t device, int field, int size) {
	outportl(0xCF8, (uint64_t)pcie_addr(device, field));

	if (size == 4) {
		uint32_t t = inportl(0xCFC);
		return t;
	} else if (size == 2) {
		uint16_t t = inports(0xCFC + (field & 2));
		return t;
	} else if (size == 1) {
		uint8_t t = inportb(0xCFC + (field & 3));
		return t;
	}

	return 0xFFFF;
}

uint32_t pci_get_interrupt(uint32_t device)
{
	return pci_read_field(device, PCI_INTERRUPT_LINE, 1);
}

struct vfile* pci_vfsmap(struct pcidev dev)
{
	struct vfile* vf = (struct vfile*)kalloc(sizeof(struct vfile), KERN_MEM);
	vf->drno = dev.device;
	vf->mem = (void*)pci_baseaddr(0, dev);
	vf->refcnt = 1;
	return vf;
}

uint16_t pci_find_type(uint32_t dev) {
	struct pcidev d = {.device = dev};
	return (pciget(d, PCI_CLASS) << 8) | pciget(d, PCI_SUBCLASS);
}
