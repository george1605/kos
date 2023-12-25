#ifndef __SUPERIO__
#define __SUPERIO__
#include "../pci.h"
#include "../smem.h"
#include "../irq.c"
#include "../driv.h"
#define FANS 0
#define VOLTAGES 1
#define TEMPS 2
#define PC87360_REG_PRESCALE(nr)	(0x00 + 2 * (nr))
#define PC87360_REG_PWM(nr)		    (0x01 + 2 * (nr))
#define PC87360_REG_FAN_MIN(nr)		(0x06 + 3 * (nr))
#define PC87360_REG_FAN(nr)		    (0x07 + 3 * (nr))
#define PC87360_REG_FAN_STATUS(nr)	(0x08 + 3 * (nr))
static const u8 logdev[3] = { [FANS] = 0x9, [VOLTAGES] = 0xe, [TEMPS] = 0xf };

enum {
    SUPERIO_INDEX0 = 0x2E,
    SUPERIO_DATA0  = 0x2F,
    SUPERIO_INDEX1 = 0x4E,
    SUPERIO_DATA1  = 0x4F
};

enum {
    SUPERIO_ID = 0x20,
    SUPERIO_CONF1,
    SUPERIO_CONF2,
    SUPERIO_CONF3,
    SUPERIO_CONF4,
    SUPERIO_CONF5,
    SUPERIO_REVID = 0x27,
    SUPERIO_CONF8,
    SUPERIO_CONFA = 0x2A,
    SUPERIO_CONFB,
    SUPERIO_CONFC,
    SUPERIO_CONFD,
    SUPERIO_BASE = 0x60
};

inline void pc87363_outb(int sioaddr, int reg, int val)
{
	outb(reg, sioaddr);
	outb(val, sioaddr + 1);
}

inline int pc87363_inb(int sioaddr, int reg)
{
	outb(reg, sioaddr);
	return inb(sioaddr + 1);
}

void pc87363_exit(int sioaddr) 
{
	outb(0x02, sioaddr);
	outb(0x02, sioaddr + 1);
}

void* pc87363_addr(int sioaddr)
{
    uint64_t val;
    val = (pc87363_inb(sioaddr, SUPERIO_BASE) << 8)
		    | pc87363_inb(sioaddr, SUPERIO_BASE + 1);
    return (void*)val;
}

struct vfile* pc87363_map()
{
    struct vfile* vf = (struct vfile*)kalloc(sizeof(struct vfile), KERN_MEM);
    vf->mem = pc87363_addr(SUPERIO_DATA0);
    vf->name = strdup("PC87363");
    return vf;
}

void pc87363_setup(struct drivobj* obj)
{
    obj->version = VERSION(1,0,0); // version 1.0.0
    obj->name = strdup("PC87363");
}

void pc87363_main()
{
    EXPORT_DRIVER(pc87363_setup);
}
#endif