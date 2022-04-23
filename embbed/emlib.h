#pragma once
#ifndef HIGH
#define HIGH 1
#define LOW 0
#endif
#define AVR_SYS 0
#define MPC_SYS 1
#define RPI_SYS 2
#define INTR(x) __attribute__((interrupt(x)))
typedef unsigned int dword;
typedef unsigned short word;

#ifdef __ARM__

void INTR("IRQ") inthnd()
{
    
}

#endif

void __main_start();