// HDMI support
#pragma once
#include "ioctl.h"
#define HDMI_INPUT_8BIT 0
#define HDMI_INPUT_10BIT 1
#define HDMI_INPUT_12BIT 2
#define HDMI_IRQ_CORE (1 << 0)
#define HDMI_IRQ_OCP_TIMEOUT (1 << 4)
#define HDMI_IRQ_AUDIO_FIFO_UNDERFLOW (1 << 8)
#define HDMI_IRQ_AUDIO_FIFO_OVERFLOW (1 << 9)
#define HDMI_IRQ_AUDIO_FIFO_SAMPLE_REQ (1 << 10)
#define HDMI_IRQ_VIDEO_VSYNC (1 << 16)
#define HDMI_IRQ_VIDEO_FRAME_DONE (1 << 17)
#define HDMI_IRQ_PHY_LINE5V_ASSERT (1 << 24)
#define HDMI_IRQ_LINK_CONNECT (1 << 25)
#define HDMI_IRQ_LINK_DISCONNECT (1 << 26)
#define HDMI_IRQ_PLL_LOCK (1 << 29)
#define HDMI_IRQ_PLL_UNLOCK (1 << 30)
#define HDMI_IRQ_PLL_RECAL (1 << 31)

struct fb_cmap_user
{
	size_t start;
	size_t len;
	uint16_t *red;
	uint16_t *green;
	uint16_t *blue;
	uint16_t *transp;
};

struct fb_info
{
	char *smem;
	size_t xres;
	size_t yres;
	size_t xoff;
	size_t yoff;
	char gray;
};

struct fb_image_user
{
	size_t dx;
	size_t dy;
	size_t width;
	size_t height;
	size_t fg_color;
	size_t bg_color;
	uint8_t depth;
	const char *data;
	struct fb_cmap_user cmap; /* color map info */
};

struct fb_cursor_user
{
	uint16_t set;
	uint16_t enable;
	uint16_t rop;
	const char *mask;
	// struct fbcurpos hot;
	struct fb_image_user image;
};

struct fb_blit_caps
{
	size_t x;
	size_t y;
	size_t len;
	size_t flags;
};

struct fb_dev
{
	int num;
	int flags;
	int type;
	char *buffer;
};

#define FBIO_CURSOR IOWR('F', 0x08, struct fb_cursor_user)

#define fb_readb(addr) (*(volatile uint8_t *)(addr))
#define fb_readw(addr) (*(volatile uint16_t *)(addr))
#define fb_readl(addr) (*(volatile size_t *)(addr))
#define fb_writeb(b, addr) (*(volatile uint8_t *)(addr) = (b))
#define fb_writew(b, addr) (*(volatile uint16_t *)(addr) = (b))
#define fb_writel(b, addr) (*(volatile size_t *)(addr) = (b))

enum hdmi_infotype
{
	HDMI_TYPE_VENDOR = 0x81,
	HDMI_TYPE_AVI = 0x82,
	HDMI_TYPE_SPD = 0x83,
	HDMI_TYPE_AUDIO = 0x84,
};

enum intelregs
{
	PipeAConf = 0x70008,
	PipeBConf = 0x71008,
	GMBusData = 0x510C,
	GMBusStatus = 0x5108,
	GMBusCommand = 0x5104,
	GMBusClock = 0x5100,
	DisplayPlaneAControl = 0x70180,
	DisplayPlaneALinearOffset = 0x70184,
	DisplayPlaneAStride = 0x70188,
	DisplayPlaneASurface = 0x7019C,
	DPLLDivisorA0 = 0x6040,
	DPLLDivisorA1 = 0x6044,
	DPLLControlA = 0x6014,
	DPLLControlB = 0x6018,
	DPLLMultiplierA = 0x601C,
	HTotalA = 0x60000,
	HBlankA = 0x60004,
	HSyncA = 0x60008,
	VTotalA = 0x6000C,
	VBlankA = 0x60010,
	VSyncA = 0x60014,
	PipeASource = 0x6001C,
	AnalogDisplayPort = 0x61100,
	VGADisplayPlaneControl = 0x71400,
};

/*
	Generic Framebuffer functionality
	To be used by screen.h
*/

#define GET_ALPHA(color) ((color >> 24) & 0x000000FF)
#define GET_RED(color)   ((color >> 16) & 0x000000FF)
#define GET_GREEN(color) ((color >> 8)  & 0x000000FF)
#define GET_BLUE(color)  ((color >> 0)  & 0X000000FF)

struct
{
	uint16_t res_x;
	uint16_t res_y;
	uint32_t res_b;
	uint32_t res_s;
	void *mem;
} fb_info;

struct fb_rect
{
	uint16_t x1, x2, y1, y2;
};

#define FONT_UTF8 0x1
#define FONT_UNICODE 0x2

struct fb_font
{
	uint16_t std; // UTF-8 or Unicode
	uint16_t height;
	uint16_t width;
	void(*get_char)(struct fb_font* font, uint8_t* buffer, uint16_t c);
	uint8_t glyph[];
};

typedef struct {
    float x, y, z;
} fb_point3d;

typedef struct {
    float x, y;
} fb_point2d;

struct {
    float focal;
    float angle;
} fb_camera;

void fb_project(fb_point3d input, fb_point2d *output, float focalLength) {
    output->x = (input.x * focalLength) / input.z;
    output->y = (input.y * focalLength) / input.z;
}

void fb_rotate(fb_point2d input, fb_point2d* output, float angle) {
	output->x = input.x * cos(angle) - sin(angle) * input.y;
	output->y = input.x * sin(angle) + cos(angle) * input.y;
}

void bitmap_getchar(struct fb_font* font, uint8_t* buffer, uint16_t c)
{
	int index = font->height * font->width * (c - 'A');
	for(int i = 0;i < font->height * font->width;i++)
		buffer[i] = ((font->glyph[i] == 1) ? 0xffffff : 0x000000);
}

void fb_clear(uint32_t color)
{
	if (fb_info.mem == NULL_PTR)
		return;
	size_t size = fb_info.res_x * fb_info.res_y * fb_info.res_b / 8;
	memset(fb_info.mem, color, size);
}

void fb_set_pixel(uint16_t x, uint16_t y, uint32_t color)
{
	((uint32_t *)fb_info.mem)[y * fb_info.res_x + x] = color;
}

uint32_t fb_get_pixel(uint16_t x, uint16_t y)
{
	void* index = fb_info.mem + (y * fb_info.res_x + x) * 4;
	return *(uint32_t*)index;
}

uint32_t fb_get_pixelx(uint16_t x, uint16_t y)
{
	void* index = fb_info.mem + (y * fb_info.res_x + x) * (fb_info.res_b / 8);
	switch(fb_info.res_b)
	{
	case 32:
		return fb_readl(index);
	case 16:
		return fb_readw(index);
	case 8:
		return fb_readb(index);
	default:
		return 0;
	}
}

void fb_set_pixelx(uint16_t x, uint16_t y, uint32_t value)
{
	void* index = fb_info.mem + (y * fb_info.res_x + x) * (fb_info.res_b / 8);
	switch(fb_info.res_b)
	{
	case 32:
		*(uint32_t*)index = value;
	case 16:
		*(uint16_t*)index = value;
	case 8:
		*(uint8_t*)index = value;
	default:
		// nothing();
	}
}

void fb_capture(uint32_t *buffer, struct fb_rect info)
{
	size_t width = info.x2 - info.x1;
	for (int i = info.x1; i <= info.x2; i++)
		for (int j = info.y1; j <= info.y2; j++)
			buffer[j * width + i] = ((uint32_t *)fb_info.mem)[j * width + i];
}

void fb_copy(uint32_t *buffer, struct fb_rect info)
{
	size_t width = info.x2 - info.x1;
	for (int i = info.x1; i <= info.x2; i++)
		for (int j = info.y1; j <= info.y2; j++)
			((uint32_t *)fb_info.mem)[j * width + i] = buffer[j * width + i];
}

void fb_write_char(struct fb_font* font, uint16_t ch, struct fb_rect rect)
{
	if(ch > 0xFF && font->std != FONT_UNICODE) return;
	uint32_t* buffer = kalloc(font->width * font->height * 4, KERN_MEM);
	font->get_char(font, buffer, ch);
	fb_copy(buffer, rect);
}

inline uint32_t fb_lerp(float f, uint32_t col1, uint32_t col2)
{
	return col1 + (col2 - col1) * f;
}

inline uint32_t fb_alpha_merge(uint32_t col1, uint32_t col2)
{
	uint8_t alpha = GET_ALPHA(col1);
	return fb_lerp((float)alpha / 255.0f, col1, col2);
}

// Optimised it so it performs division only once, not width * height 
void fb_gradient(uint32_t col1, uint32_t col2, struct fb_rect rect)
{
	uint32_t last;
	uint16_t rectsz = rect.x2 - rect.x1;
	float div = (float)1 / (float)rectsz, need = 0.0f;
	for (int i = rect.x1; i <= rect.x2; i++)
	{
		for (int j = rect.y1; j <= rect.y2; j++)
		{
			((uint32_t*)fb_info.mem)[j * rectsz + i] = last;
		}
		last = fb_lerp(col1, col2, need);
		need += div;
	}
}

void fb_line_bres(int x1, int y1, int x2, int y2, uint32_t color)
{
  int dx, dy, p, x, y;
  dx = x2 - x1;
  dy = y2 - y1;
  x = x1;
  y = y1;
  p = 2 * dy - dx;
  while (x < x2)
  {
    if (p >= 0)
    {
      ((uint32_t*)fb_info.mem)[y * fb_info.res_x + x] = color;
      y = y + 1;
      p = p + 2 * dy - 2 * dx;
    }
    else
    {
      ((uint32_t*)fb_info.mem)[y * fb_info.res_y + x] = color;
      p = p + 2 * dy;
    }
    x = x + 1;
  }
}
