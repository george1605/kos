#pragma once
#include "../../fs.h"
#include "../../pci.h"
#include "../../smem.h"
#include "../fb.h"
#define REG_PIPEASRC      0x6001C
#define REG_PIPEACONF     0x70008
#define PIPEACONF_ENABLE (1 << 31)
#define PIPEACONF_STATE  (1 << 30)
#define REG_DSPALINOFF    0x70184
#define REG_DSPASTRIDE    0x70188
#define REG_DSPASURF      0x7019c

extern ext2_gen_device* lfb_device;
extern int lfb_use_write_combining;
static uint64_t ctrl_regs = 0;

static uint32_t i965_mmio_read(uint32_t reg) {
	return *(volatile uint32_t*)(ctrl_regs + reg);
}

static void i965_mmio_write(uint32_t reg, uint32_t val) {
	*(volatile uint32_t*)(ctrl_regs + reg) = val;
}

static void split(uint32_t val, uint32_t * a, uint32_t * b) {
	*a = (val & 0xFFFF) + 1;
	*b = (val >> 16) + 1;
}

static void i965_modeset(uint16_t x, uint16_t y) {
	/* Disable pipe A while we update source size */
	uint32_t pipe = i965_mmio_read(REG_PIPEACONF);
	i965_mmio_write(REG_PIPEACONF, pipe & ~PIPEACONF_ENABLE);
	while (i965_mmio_read(REG_PIPEACONF) & PIPEACONF_STATE);

	/* Set source size */
	i965_mmio_write(REG_PIPEASRC, ((x - 1) << 16) | (y - 1));

	/* Re-enable pipe */
	pipe = i965_mmio_read(REG_PIPEACONF);
	i965_mmio_write(REG_PIPEACONF, pipe | PIPEACONF_ENABLE);
	while (!(i965_mmio_read(REG_PIPEACONF) & PIPEACONF_STATE));

	/* Keep the plane enabled while we update stride value */
	i965_mmio_write(REG_DSPALINOFF, 0);        /* offset to default of 0 */
	i965_mmio_write(REG_DSPASTRIDE, x * 4); /* stride to 4 x width */
	i965_mmio_write(REG_DSPASURF, 0);          /* write to surface address triggers change; use default of 0 */

	/* Update the values we expose to userspace. */
	fb_info.res_x = x;
	fb_info.res_y = y;
	fb_info.res_b = 32;
	fb_info.res_s = i965_mmio_read(REG_DSPASTRIDE);
	size_t lfb_memsize = fb_info.res_s * fb_info.res_y;
	lfb_device->offset  = lfb_memsize;
}

static void setup_framebuffer(uint32_t pcidev) {
	/* Map BAR space for the control registers */
	uint32_t ctrl_space = pci_read_field(pcidev, PCI_BAR0, 4);
	pci_write_field(pcidev, PCI_BAR0, 4, 0xFFFFFFFF);
	uint32_t ctrl_size = pci_read_field(pcidev, PCI_BAR0, 4);
	ctrl_size = ~(ctrl_size & -15) + 1;
	pci_write_field(pcidev, PCI_BAR0, 4, ctrl_space);
	ctrl_space &= 0xFFFFFF00;
	ctrl_regs = (uint64_t)map_page(ctrl_space, ctrl_size, PAGE_PRESENT | PAGE_WRITABLE);

	i965_modeset(1440,900);

	lfb_use_write_combining = 1;
	memset(fb_info.mem, 0, fb_info.res_x * fb_info.res_y * fb_info.res_b / 8);
}

static void find_i965(uint32_t device, uint16_t v, uint16_t d, void * extra) {
	if (v == 0x8086 && d == 0x0046) {
		setup_framebuffer(device);
	}
}

int i965_setup()
{
    for(int i = 0;i < pcinum;i++)
        if(pci_devs[i].vendor == 0x8086 && pci_devs[i].device == 0x0046) {
            setup_framebuffer(pci_devs[i].device);
			return 0;
		}
	return 1;
}