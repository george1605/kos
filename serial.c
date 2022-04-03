#pragma once
#include "port.h"

void irqon_8259(size_t irq)
{
	if(irq >= 16)
		return;
	if(irq >= 8)
	{
		outportb(0x21, inportb(0x21) & ~0x04);
		irq -= 8;
		irq = 1 << irq;
		outportb(0xA1, inportb(0xA1) & ~irq);
		return;
	}
	irq = 1 << irq;
	outportb(0x21, inportb(0x21) & ~irq);
}

void irqoff_8259(size_t irq)
{
	if(irq >= 16)
		return;
	if(irq >= 8)
	{
		/* outportb(0x21, inportb(0x21) | 0x04); */
		irq -= 8;
		irq = 1 << irq;
		outportb(0xA1, inportb(0xA1) | irq);
		return;
	}
	irq = 1 << irq;
	outportb(0x21, inportb(0x21) | irq);
}

