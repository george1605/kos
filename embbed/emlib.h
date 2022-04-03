#pragma once
#ifndef HIGH
#define HIGH 1
#define LOW 0
#endif
#define AVR_SYS 0
#define MPC_SYS 1
#define RPI_SYS 2
typedef unsigned int dword;
typedef unsigned short word;

void setsys(int systype)
{
}

void __main_start();