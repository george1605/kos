// PORT of Olive to the KOS enivronment
#pragma once
#include "../stdlib.c" // <-- It is USERSPACE!!!

typedef unsigned char u8;
typedef unsigned int u32;
typedef struct { uint16_t x, y; } OlPoint;

typedef struct {
  u32* front, back;
  u32 w, h;
  int mono; // if is monochrome
} OlWindow;

typedef struct {
  u8 r, g, b, a;
} OlColor;

int ol_save_ppm(char* fname, OlWindow win)
{
  FILE *fp = _fopen(fname, "wb");
  _fprintf(fp, "P6\n%zu %zu 255\n", win.w, win.h);
  int res = 0;
  for (size_t i = 0; i < win.w * win.h; ++i)
  {
    uint32_t pixel = win.front[i];
    uint8_t bytes[3] = {
        (pixel >> (8 * 0)) & 0xFF,
        (pixel >> (8 * 1)) & 0xFF,
        (pixel >> (8 * 2)) & 0xFF,
    };
    _fwrite(bytes, sizeof(bytes), 1, fp);
    if (_ferror(fp))
    {
      res = errno;
      goto err;
    }
  }
err:
  if(fp) _fclose(fp);
  return res;
}

