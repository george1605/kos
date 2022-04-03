#pragma once
#include "port.h"

void nmion() {
    outb(0x70, inb(0x70) & 0x7F);
    inb(0x71);
 }
 
 void nmioff() {
    outb(0x70, inb(0x70) | 0x80);
    inb(0x71);
 }
