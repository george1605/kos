#pragma once
#include "../port.h"
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
  delay(30);
}