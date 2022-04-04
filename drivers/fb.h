//HDMI support
#include "ioctl.h"
#define HDMI_INPUT_8BIT  0
#define HDMI_INPUT_10BIT 1
#define	HDMI_INPUT_12BIT 2
#define HDMI_IRQ_CORE				(1 << 0)
#define HDMI_IRQ_OCP_TIMEOUT			(1 << 4)
#define HDMI_IRQ_AUDIO_FIFO_UNDERFLOW	(1 << 8)
#define HDMI_IRQ_AUDIO_FIFO_OVERFLOW	(1 << 9)
#define HDMI_IRQ_AUDIO_FIFO_SAMPLE_REQ	(1 << 10)
#define HDMI_IRQ_VIDEO_VSYNC			(1 << 16)
#define HDMI_IRQ_VIDEO_FRAME_DONE		(1 << 17)
#define HDMI_IRQ_PHY_LINE5V_ASSERT		(1 << 24)
#define HDMI_IRQ_LINK_CONNECT			(1 << 25)
#define HDMI_IRQ_LINK_DISCONNECT		(1 << 26)
#define HDMI_IRQ_PLL_LOCK			(1 << 29)
#define HDMI_IRQ_PLL_UNLOCK			(1 << 30)
#define HDMI_IRQ_PLL_RECAL			(1 << 31)

struct fb_cmap_user {
	size_t start;			
	size_t len;			
	uint16_t* red;		
	uint16_t* green;
	uint16_t* blue;
	uint16_t* transp;		
};

struct fb_info {
	char* smem;
	size_t xres;
	size_t yres;
	size_t xoff;
	size_t yoff;
	char gray;
};

struct fb_image_user {
	size_t dx;			
	size_t dy;
	size_t width;		
	size_t height;
	size_t fg_color;
	size_t bg_color;
	uint8_t  depth;			
	const char* data;	
	struct fb_cmap_user cmap;	/* color map info */
};

struct fb_cursor_user {
	uint16_t set;			
	uint16_t enable;			
	uint16_t rop;			
	const char *mask;
	struct fbcurpos hot;		
	struct fb_image_user image;	
};

struct fb_blit_caps {
	size_t x;
	size_t y;
	size_t len;
	size_t flags;
};

struct fb_dev {
	int num;
	int flags;
	int type;
	char* buffer;
};

#define FBIO_CURSOR   IOWR('F', 0x08, struct fb_cursor_user)

#define fb_readb(addr) (*(volatile uint8_t *) (addr))
#define fb_readw(addr) (*(volatile uint16_t *) (addr))
#define fb_readl(addr) (*(volatile size_t *) (addr))
#define fb_writeb(b,addr) (*(volatile uint8_t *) (addr) = (b))
#define fb_writew(b,addr) (*(volatile uint16_t *) (addr) = (b))
#define fb_writel(b,addr) (*(volatile size_t *) (addr) = (b))

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