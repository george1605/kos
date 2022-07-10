#pragma once
#include "../lib.c"
#undef NOP
#undef ASMV
#undef HALT
#define RPI_RLIMIT (1 << 30)
static size_t MMIO_BASE;
 
static inline void mmio_init(int raspi){
    switch (raspi) {
        case 2:
        case 3:  MMIO_BASE = 0x3F000000; break; // for raspi2 & 3
        case 4:  MMIO_BASE = 0xFE000000; break; // for raspi4
        default: MMIO_BASE = 0x20000000; break; // for raspi1, raspi zero etc.
    }
}

static inline void delay(int count){
	asm volatile("__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n"
		 : "=r"(count): [count]"0"(count) : "cc");
}

 
enum RPI_IO {
    GPIO_BASE = 0x200000, 
    GPPUD = (GPIO_BASE + 0x94),
    GPPUDCLK0 = (GPIO_BASE + 0x98),
    UART0_BASE = (GPIO_BASE + 0x1000), 
 
    // The offsets for reach register for the UART.
    UART0_DR     = (UART0_BASE + 0x00),
    UART0_RSRECR = (UART0_BASE + 0x04),
    UART0_FR     = (UART0_BASE + 0x18),
    UART0_ILPR   = (UART0_BASE + 0x20),
    UART0_IBRD   = (UART0_BASE + 0x24),
    UART0_FBRD   = (UART0_BASE + 0x28),
    UART0_LCRH   = (UART0_BASE + 0x2C),
    UART0_CR     = (UART0_BASE + 0x30),
    UART0_IFLS   = (UART0_BASE + 0x34),
    UART0_IMSC   = (UART0_BASE + 0x38),
    UART0_RIS    = (UART0_BASE + 0x3C),
    UART0_MIS    = (UART0_BASE + 0x40),
    UART0_ICR    = (UART0_BASE + 0x44),
    UART0_DMACR  = (UART0_BASE + 0x48),
    UART0_ITCR   = (UART0_BASE + 0x80),
    UART0_ITIP   = (UART0_BASE + 0x84),
    UART0_ITOP   = (UART0_BASE + 0x88),
    UART0_TDR    = (UART0_BASE + 0x8C),
 
    MBOX_BASE    = 0xB880,
    MBOX_READ    = (MBOX_BASE + 0x00),
    MBOX_STATUS  = (MBOX_BASE + 0x18),
    MBOX_WRITE   = (MBOX_BASE + 0x20)
};
 

void uart_init(int raspi){
	mmio_init(raspi);
 
	mmio_write(UART0_CR, 0x00000000);
 	mmio_write(GPPUD, 0x00000000);
	delay(150);
 
	mmio_write(GPPUDCLK0, (1 << 14) | (1 << 15));
	delay(150);

	mmio_write(GPPUDCLK0, 0x00000000);
	mmio_write(UART0_ICR, 0x7FF);
 
	if (raspi >= 3) {
		// UART_CLOCK = 30000000;
		unsigned int r = (((unsigned int)(&mbox) & ~0xF) | 8);
		// wait until we can talk to the VC
		while ( mmio_read(MBOX_STATUS) & 0x80000000 ) { }
		// send our message to property channel and wait for the response
		mmio_write(MBOX_WRITE, r);
		while ( (mmio_read(MBOX_STATUS) & 0x40000000) || mmio_read(MBOX_READ) != r ) { }
	}
 
	// Divider = 3000000 / (16 * 115200) = 1.627 = ~1.
	mmio_write(UART0_IBRD, 1);
	// Fractional part register = (.627 * 64) + 0.5 = 40.6 = ~40.
	mmio_write(UART0_FBRD, 40);
	mmio_write(UART0_LCRH, (1 << 4) | (1 << 5) | (1 << 6));
	mmio_write(UART0_IMSC, (1 << 1) | (1 << 4) | (1 << 5) | (1 << 6) |
	                       (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10));
 
	mmio_write(UART0_CR, (1 << 0) | (1 << 8) | (1 << 9));
}

void uart_putc(uint8_t c)
{
	while ( mmio_read(UART0_FR) & (1 << 5) ) { 

    }
	mmio_write(UART0_DR, c);
}

void uart_puts(char* u){
    int i;
    for(i = 0;u[i] != 0;i++)
     uart_putc((uint8_t)u[i]);
}

void krpi_main(){
    uart_init(2);
    uart_puts("Hello Kernel!");
    while(1){
        NOP();
    }
}

// some PICO Things
#define PI3_BREAK asm volatile("brk  #0")
#define PICO_GPIO_BASE 0x3F200000
