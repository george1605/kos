#pragma once
#define SB16_CARD 2
#define INTEL_CARD 4
#define REALTEK_CARD 16
#define OTHER_CARD 256
#include "../port.h"
#include "../speaker.h"

int getsoundinfo(){
 int u = inb(0x200);
 return u;
}

void testsound(){
 
}
