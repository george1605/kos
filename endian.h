#pragma once
#include "lib.c"
uint16_t switch_endian16(uint16_t nb) {
       return (nb>>8) | (nb<<8);
}
   
size_t switch_endian32(size_t nb) {
       return ((nb>>24)&0xff)      |
              ((nb<<8)&0xff0000)   |
              ((nb>>8)&0xff00)     |
              ((nb<<24)&0xff000000);
}
