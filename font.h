#pragma once 
#include "screen.h"
struct font {
  char** fmap;
  int id;
} deffont;

char cursor[16][16] = {
  {1, 0, 0, 0, 0, 0},
  {1, 1, 0, 0, 0, 0},
  {1, 1, 1, 0, 0, 0},
  {1, 1, 1, 1, 0, 0},
  {1, 1, 1, 1, 1, 0},
  {1, 1, 1, 1, 1, 1},
  {0, 0, 1, 1, 0, 0},
  {0, 0, 1, 1, 0, 0},
  {0, 0, 1, 1, 0, 0}
};
