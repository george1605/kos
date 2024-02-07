#include "system.h"

struct gdt_entry
{
    unsigned short limit_low;
    unsigned short base_low;
    unsigned char base_middle;
    unsigned char access;
    unsigned char granularity;
    unsigned char base_high;
} __attribute__((packed));

#define SEG(limlow, baselow, basemid, basehig)                                                 \
    {                                                                                          \
        .limit_low = limlow, .base_low = baselow, .base_middle = basemid, .base_high = basehig \
    }

struct gdt_ptr
{
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));

struct gdt_entry gdt[3];
struct gdt_ptr gp;
struct taskstate tss;
extern void gdt_flush();

void gdt_set_gate(int num, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran)
{
    /* Setup the descriptor base address */
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;
    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F);

    /* Finally, set up the granularity and access flags */
    gdt[num].granularity |= (gran & 0xF0);
    gdt[num].access = access;
}

void gdt_install()
{
    gp.limit = (sizeof(struct gdt_entry) * 3) - 1;
    gp.base = &gdt;

    gdt_set_gate(0, 0, 0, 0, 0);
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
    gdt_set_gate(3, (uint32_t)&tss, sizeof tss, 0x89, 0x80);
    gdt_flush();
}

void gdt_init()
{
    gdt_install();
}

void lgdt(struct gdt_entry *p, int size)
{
    volatile uint16_t pd[3];

    pd[0] = size - 1;
    pd[1] = (size_t)p;
    pd[2] = (size_t)p >> 16;

    asm volatile("lgdt (%0)"
                 :
                 : "r"(pd));
}