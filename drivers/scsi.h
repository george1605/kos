#pragma once
#include "../port.h"
#include "../fs.h"
#define SE2CS       0x08
#define SE2CLK      0x04
#define SE2DO       0x02
#define SE2DI       0x01
#define TUL_NVRAM   0x5D

void udelay(int u){
  int a;
  for(a = 0;a < u;a++){}
}

void initio_se2(unsigned long base, uint8_t instr){
  int i;
  uint8_t b;

  outb(SE2CS | SE2DO, base + TUL_NVRAM);		
  udelay(30);
  outb(SE2CS | SE2CLK | SE2DO, base + TUL_NVRAM);	
  udelay(30);

  for (i = 0; i < 8; i++) {
	 if (instr & 0x80)
		b = SE2CS | SE2DO;		
	 else
		b = SE2CS;			
	 outb(b, base + TUL_NVRAM);
	 udelay(30);
	 outb(b | SE2CLK, base + TUL_NVRAM);	   
	 udelay(30);
	 instr <<= 1;
  }
  outb(SE2CS, base + TUL_NVRAM);			
  udelay(30);
}

static u16 initio_se2_rd(unsigned long base, u8 addr)
{
	u8 instr, rb;
	u16 val = 0;
	int i;

	instr = (u8)(addr | 0x80);
	initio_se2(base, instr);	/* READ INSTR */

	for (i = 15; i >= 0; i--) {
		outb(SE2CS | SE2CLK, base + TUL_NVRAM);	/* +CLK */
		udelay(30);
		outb(SE2CS, base + TUL_NVRAM);		/* -CLK */

		/* sample data after the following edge of clock  */
		rb = inb(base + TUL_NVRAM);
		rb &= SE2DI;
		val += (rb << i);
		udelay(30);	/* 6/20/95 */
	}

	outb(0, base + TUL_NVRAM);		/* no chip select */
	udelay(30);
	return val;
}

static void initio_se2_wr(unsigned long base, u8 addr, u16 val)
{
	u8 rb;
	u8 instr;
	int i;

	instr = (u8) (addr | 0x40);
	initio_se2(base, instr);	/* WRITE INSTR */
	for (i = 15; i >= 0; i--) {
		if (val & 0x8000)
			outb(SE2CS | SE2DO, base + TUL_NVRAM);	/* -CLK+dataBit 1 */
		else
			outb(SE2CS, base + TUL_NVRAM);		/* -CLK+dataBit 0 */
		udelay(30);
		outb(SE2CS | SE2CLK, base + TUL_NVRAM);		/* +CLK */
		udelay(30);
		val <<= 1;
	}
	outb(SE2CS, base + TUL_NVRAM);				/* -CLK */
	udelay(30);
	outb(0, base + TUL_NVRAM);				/* -CS  */
	udelay(30);

	outb(SE2CS, base + TUL_NVRAM);				/* +CS  */
	udelay(30);

	for (;;) {
		outb(SE2CS | SE2CLK, base + TUL_NVRAM);		/* +CLK */
		udelay(30);
		outb(SE2CS, base + TUL_NVRAM);			/* -CLK */
		udelay(30);
		if ((rb = inb(base + TUL_NVRAM)) & SE2DI)
			break;	/* write complete */
	}
	outb(0, base + TUL_NVRAM);				/* -CS */
}

