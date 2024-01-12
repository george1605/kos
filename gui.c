#pragma once
#include "screen.h"
#include "mem.h"
#include "font.h"
size_t titlebar_colors[26] = {0xff59584f, 0xff5f5d53, 0xff58564e, 0xff57554d, 0xff56544c, 0xff55534b,
                              0xff54524a, 0xff525049, 0xff514f48, 0xff504e47, 0xff4e4c45, 0xff4e4c45,
                              0xff4c4a44, 0xff4b4943, 0xff4a4842, 0xff484741, 0xff46453f, 0xff45443f,
                              0xff44433e, 0xff43423d, 0xff42413c, 0xff403f3a, 0xff3f3e3a, 0xffb0123f};
#define BORDER_FLAT 2
#define BORDER_HIGH 4 /* risen */
#define BORDER_LOW 8  /* low */

#define BUTTON_CLS 2
#define BUTTON_MIN 4
#define BUTTON_MAX 8

#define COMP_LABEL 1
#define COMP_BUTTON 2
#define COMP_CHECKBOX 3
#define COMP_MENU 4

#define MENU_ENTRY_HEIGHT 20
#define MENU_BACKGROUND RGB(239, 238, 232)
#define MENU_ICON_SIZE 16

#define HILIGHT_BORDER_TOP RGB(54, 128, 205)
#define HILIGHT_GRADIENT_TOP RGB(93, 163, 236)
#define HILIGHT_GRADIENT_BOTTOM RGB(56, 137, 220)
#define HILIGHT_BORDER_BOTTOM RGB(47, 106, 167)
typedef void *icon;
long last_wid = 0xFF0;

icon LoadIcon(char *fname)
{
  return (void *)0;
}

long CreateWID()
{ /* creates a window id */
  last_wid += 0xA;
  return last_wid;
}

void MsgBox(char *text, char *title, int flags)
{
  switch (flags)
  {
  case 1:

    break;
  }
}

void vidsetup()
{
  fbsetres(1920, 1080);
  void *u = getfb();
  if (u == 0)
    perror("Cannot setup Graphics Mode");
}

struct rect
{
  size_t x;
  size_t y;
  size_t width;
  size_t height;
} _panel;

struct dfb
{
  size_t width;
  size_t height;
  char *front;
  char *back;
};

void fbswap(struct dfb *u)
{
  char *i = u->back;
  u->back = u->front;
  u->front = i;
  i = 0;
}

void DrawRect(struct rect u, size_t color)
{
  int x;
  int y;
  for (x = u.x; x < (u.x + u.width); x++)
    for (y = u.y; y < (u.y + u.height); y++)
      *(VBUFFER + 320 * y + x) = color;
}

void DrawRectX(struct rect u, size_t color)
{ // GENERIC VBE MODES
  size_t *screen = (char *)gfxmode->physical_base_pointer;
  int x, y, loc;
  for (x = u.x; x < (u.x + u.width); x++)
  {
    for (y = u.y; y < (u.y + u.height); y++)
    {
      loc = y * gfxmode->x_resolution + x;
      screen[loc] = color;
    }
  }
}

void DrawRectC(int x, int y, int width, int height)
{
  struct rect r;
  r.x = x;
  r.y = y;
  r.width = width;
  r.height = height;
  DrawRectX(r, vbecolor);
}

void SetPixelX(int x, int y, size_t color)
{
  int loc = y * 1920 + x;
  char *b = (char *)gfxmode->physical_base_pointer;
  b[loc] = color;
}

struct titlebar
{
  size_t bkcolor;
  size_t color;
  struct rect area;
  char *text;
};

struct taskbar
{
  size_t bkcolor;
  size_t color;
  struct rect area;
  char *text;
};

void DrawTitleBar(struct titlebar u)
{
  DrawRectX(u.area, u.bkcolor);
}

struct line
{
  size_t x1;
  size_t y1;
  size_t x2;
  size_t y2;
};

struct pallette
{
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t a;
};

struct pos
{
  size_t x;
  size_t y;
} turtle;

struct canvas
{
  int width;
  int height;
  uint32_t *framebuffer;
};

struct window
{
  char *back; // background pixels
  char *front;
  struct rect area;
  struct titlebar tbar;
  int active;
  long id;
};

struct wm // Window Manager
{
  char* name;
  struct vscreen* scrn;
  struct window*(*create)(char* name, int, int, int, int);
  void(*remove)(struct window* win);
  void(*move)(struct window* win, int, int);
  void(*resize)(struct window* win, int, int);
  void(*show)(struct window* win);
  void(*hide)(struct window* win);
} wmout; // <- we use this

void kos_wm(struct wm* out)
{
  out->name = "KOSWM";
  out->scrn = scrn_req(0);
}

void kos_createwin(char* name, int x, int y, int w, int h)
{
  if(w <= 0 || h <= 0) return;
  if(!wmout.create)
    kos_wm(&wmout);
  wmout.create(name, x, y, w, h);
}

void kos_removewin(struct window* win)
{
  if(win == NULL_PTR)
    return;
  if(!wmout.create)
    kos_wm(&wmout);
  wout.remove(win);
}

/* defined in console.h (not this below)
struct console
{
  struct window win;
  struct proc process;
  size_t bkcolor;
  size_t color;
};
*/

struct rect InnerRect(struct window u)
{ // the inner rectangle - excluding titlebar
  struct rect i;
  i.width = u.area.width;
  i.height = u.area.height;
  i.x = u.area.x;
  i.y = u.area.y + u.tbar.area.height;
  return i;
}

char *GetWinName(struct window u)
{
  return u.tbar.text;
}

struct winstyle
{
  icon smicon;
  int border; /* border flags */
  int buttons;
};

struct comp
{
  void *bmap;
  char *text;
  int type;
};

typedef union
{
  struct comp *cmp;
  struct window *win;
} elem;

typedef struct
{
  unsigned int width;
  unsigned int height;
  char *image_bytes;
  char *buf;
  unsigned int total_size;
  uint32_t bpp;
} bitmap;

void PopBitmap(bitmap* map, void* framebuffer)
{
  if(map == NULL_PTR || framebuffer == NULL_PTR)
    return;
  map->bpp = 4;
  memmove(map->buf, framebuffer, map->width * map->height);
}

void DrawChar(char chr, struct font u)
{
}

void DrawText(char *text, struct font u)
{
  int i;
  for (i = 0; text[i] != 0; i++)
  {
    DrawChar(text[i], u);
  }
}

void SetWinName(struct window u, const char *name)
{
  if (!u.active)
  {
    u.tbar.text = (char *)name;
  }
  else
  {
  }
}

void ShowMap(bitmap *bmp)
{
  if (bmp == 0)
    return;
  uint8_t *image = bmp->image_bytes;
  size_t *screen = (char *)gfxmode->physical_base_pointer;
  int j = 0;
  int i, k;
  for (i = 0; i < bmp->height; i++)
  {
    char *image_row = image + i * bmp->width * 3;
    size_t *screen_row = (void *)screen + (bmp->height - 1 - i) * bmp->width * 4;
    j = 0;
    for (k = 0; k < bmp->width; k++)
    {
      size_t b = image_row[j++] & 0xff;
      size_t g = image_row[j++] & 0xff;
      size_t r = image_row[j++] & 0xff;
      size_t rgb = ((r << 16) | (g << 8) | (b)) & 0x00ffffff;
      rgb = rgb | 0xff000000;
      screen_row[k] = rgb;
    }
  }
}

bitmap *AllocMap(int width, int height)
{
  bitmap *u = TALLOC(bitmap);
  u->bpp = 4;
  u->width = width;
  u->height = height;
  u->image_bytes = kalloc(width * height, KERN_MEM);
  return u;
}

struct menuitem
{
  char *text;
  int type;
};

struct menu
{
  size_t bkcolor;
  size_t color;
  struct window parent;
  struct menuitem *items;
  int nitems;
};

void MoveTo(struct window u, int x, int y)
{
  turtle.x = u.area.x + x;
  turtle.y = u.area.y + y;
}

void LineTo()
{
}

void FreeMap(bitmap *u)
{
  if (u != 0)
  {
    free(u->image_bytes);
    free(u);
  }
}

void FillMap(bitmap *u, size_t color)
{
  size_t i, j;
  if (u == 0)
    return;
  for (i = 0; i < u->width; i++)
  {
    for (j = 0; j < u->height; j++)
    {
      u->image_bytes[j * u->width + i] = color;
    }
  }
}
/* from coordinates relative to the rect to global coordinates */
struct pos GlobalPos(struct pos u, struct rect i)
{
  u.x = u.x + i.x;
  u.y = u.y + i.y;
  return u;
}

void DrawMenu(struct menu i)
{
}

struct cursor
{
  bitmap *image;
  int id;
} defcrs;

void InitGUI()
{
  defcrs.image = (bitmap *)0x1CE0000;
  defcrs.id = 1;
}

bitmap *GetMap(struct window win, int bid)
{
  bitmap *u = TALLOC(bitmap);
  u->width = win.area.width;
  u->height = win.area.height;
  u->total_size = u->width * u->height * 4;
  u->bpp = 4;
  if (bid == 0)
    u->image_bytes = win.front;
  else
    u->image_bytes = win.back;
  return u;
}

void DrawWindow(struct window u)
{ // shows the titlebar and the contents
  DrawTitleBar(u.tbar);
  bitmap *i = GetMap(u, 0);
  ShowMap(i);
  FreeMap(i);
}

void DrawCursor(struct cursor i)
{
  ShowMap(i.image);
}

void DestroyWindow(struct window u)
{
  u.id = 0;
  u.active = 0;
  memset(u.front, 0, u.area.width * u.area.height);
  bitmap *i = GetMap(u, 1);
  ShowMap(i);
  FreeMap(i);
}

struct console CreateConsole(func function, size_t bkcolor, size_t color)
{
  struct console i;
  void *u = kalloc(80 * 25, USER_MEM);
  i.process = prcreat("New Console");
  i.process.stack = (char *)u;
  i.process.f = function;
  function(0, (char **)0);
}

void FreeConsole(struct console u)
{
  prkill(u.process);
  DestroyWindow(u.win);
}

void DrawCursorMap(char crs[16][16], int x, int y)
{
  int a, b;
  for (a = 0; a < 16; a++)
    for (b = 0; b < 16; b++)
      if (crs[a][b] == 1)
        SetPixelX(x, y, 0xffffffff);
}

void SendSignal(int sig_no, void *params, elem element)
{
  if (params == 0)
    return;

  switch (sig_no)
  {
  case 0xFF:
    DestroyWindow(*(struct window *)params);
    break;
  case 0xA0:
    DrawWindow(*(struct window*)params);
    break;
  case 0xB1:

    break;
  case 0xB2:

    break;
  }
}
