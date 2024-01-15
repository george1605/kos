#ifndef __SUPERIO__
#define __SUPERIO__
#include "../pci.h"
#include "../smem.h"
#include "../irq.c"
#include "../driv.h"
#include "../fs.h"
#define FANS 0
#define VOLTAGES 1
#define TEMPS 2
#define PC87360_EXTENT		0x10
#define PC87365_REG_BANK	0x09
#define PC87360_NO_BANK		0xff
#define PC87365_REG_TEMP_CONFIG		0x08
#define PC87365_REG_TEMP		    0x0B
#define PC87365_REG_TEMP_MIN		0x0D
#define PC87365_REG_TEMP_MAX		0x0C
#define PC87365_REG_TEMP_CRIT		0x0E
#define PC87365_REG_TEMP_STATUS		0x0A
#define PC87365_REG_TEMP_ALARMS		0x00
#define PC87360_REG_PRESCALE(nr)	(0x00 + 2 * (nr))
#define PC87360_REG_PWM(nr)		    (0x01 + 2 * (nr))
#define PC87360_REG_FAN_MIN(nr)		(0x06 + 3 * (nr))
#define PC87360_REG_FAN(nr)		    (0x07 + 3 * (nr))
#define PC87360_REG_FAN_STATUS(nr)	(0x08 + 3 * (nr))
#define FAN_FROM_REG(val, div)		((val) == 0 ? 0 : \
					 480000 / ((val) * (div)))
#define FAN_TO_REG(val, div)		((val) <= 100 ? 0 : \
					 480000 / ((val) * (div)))
#define FAN_DIV_FROM_REG(val)		(1 << (((val) >> 5) & 0x03))
#define FAN_STATUS_FROM_REG(val)	((val) & 0x07)

#define FAN_CONFIG_MONITOR(val, nr)	(((val) >> (2 + (nr) * 3)) & 1)
#define FAN_CONFIG_CONTROL(val, nr)	(((val) >> (3 + (nr) * 3)) & 1)
#define FAN_CONFIG_INVERT(val, nr)	(((val) >> (4 + (nr) * 3)) & 1)
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

struct pc87360_data {
	const char *name;
	ext2_gen_device *dev;
	struct spinlock lock;
	struct spinlock update_lock;
	bool valid;		/* true if following fields are valid */
	unsigned long last_updated;	/* In jiffies */

	int address[3];

	u8 fannr, innr, tempnr;

	u8 fan[3];		/* Register value */
	u8 fan_min[3];		/* Register value */
	u8 fan_status[3];	/* Register value */
	u8 pwm[3];		/* Register value */
	u16 fan_conf;		/* Configuration register values, combined */

	u16 in_vref;		/* 1 mV/bit */
	u8 in[14];		/* Register value */
	u8 in_min[14];		/* Register value */
	u8 in_max[14];		/* Register value */
	u8 in_crit[3];		/* Register value */
	u8 in_status[14];	/* Register value */
	u16 in_alarms;		/* Register values, combined, masked */
	u8 vid_conf;		/* Configuration register value */
	u8 vrm;
	u8 vid;			/* Register value */

	u8 temp[3];		/* Register value */
	u8 temp_min[3];		/* Register value */
	u8 temp_max[3];		/* Register value */
	u8 temp_crit[3];	/* Register value */
	u8 temp_status[3];	/* Register value */
	u8 temp_alarms;		/* Register value, masked */
};

static int pc87360_read_value(struct pc87360_data *data, u8 ldi, u8 bank,
			      u8 reg)
{
	int res;

	acquire(&(data->lock));
	if (bank != PC87360_NO_BANK)
		outb(bank, data->address[ldi] + PC87365_REG_BANK);
	res = inb(data->address[ldi] + reg);
	release(&(data->lock));

	return res;
}

static void pc87360_write_value(struct pc87360_data *data, u8 ldi, u8 bank,
				u8 reg, u8 value)
{
	acquire(&(data->lock));
	if (bank != PC87360_NO_BANK)
		outb(bank, data->address[ldi] + PC87365_REG_BANK);
	outb(value, data->address[ldi] + reg);
	release(&(data->lock));
}

static void pc87360_autodiv(struct pc87360_data* data, int nr)
{
	u8 old_min = data->fan_min[nr];

	/* Increase clock divider if needed and possible */
	if ((data->fan_status[nr] & 0x04) /* overflow flag */
	 || (data->fan[nr] >= 224)) { /* next to overflow */
		if ((data->fan_status[nr] & 0x60) != 0x60) {
			data->fan_status[nr] += 0x20;
			data->fan_min[nr] >>= 1;
			data->fan[nr] >>= 1;
		}
	} else {
		/* Decrease clock divider if possible */
		while (!(data->fan_min[nr] & 0x80) /* min "nails" divider */
		 && data->fan[nr] < 85 /* bad accuracy */
		 && (data->fan_status[nr] & 0x60) != 0x00) {
			data->fan_status[nr] -= 0x20;
			data->fan_min[nr] <<= 1;
			data->fan[nr] <<= 1;
		}
	}

	/* Write new fan min if it changed */
	if (old_min != data->fan_min[nr]) {
		pc87360_write_value(data, FANS, PC87360_NO_BANK,
				    PC87360_REG_FAN_MIN(nr),
				    data->fan_min[nr]);
	}
}

struct vfile* pc87363_map()
{
    struct vfile* vf = (struct vfile*)kalloc(sizeof(struct vfile), KERN_MEM);
    vf->mem = pc87363_addr(SUPERIO_DATA0);
    vf->name = strdup("PC87363");
    return vf;
}

void pc87363_setup()
{
    filesystem* fs = (filesystem*)kalloc(sizeof(filesystem), KERN_MEM);
    mount("/home/dev/pc87363", fs);
}
#endif