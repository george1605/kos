#pragma once
#include "emlib.h"
void wait(int u){
  int a;
  for(a=0;a < u;a++);
}

void blink(int portno){
  dgwrite(portno,1);
  wait(1000);
  dgwrite(portno,0);
}
