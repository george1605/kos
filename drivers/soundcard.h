#pragma once
#define SB16_CARD 2
#define INTEL_CARD 4
#define REALTEK_CARD 16
#define OTHER_CARD 256
#include "../port.h"
#include "../speaker.h"
#include "../vfs.h"

#define DSP_MIXER       0x224
#define DSP_MIXER_DATA  0x225
#define DSP_RESET       0x226
#define DSP_READ        0x22A
#define DSP_WRITE       0x22C
#define DSP_READ_STATUS 0x22E
#define DSP_ACK_8       DSP_READ_STATUS
#define DSP_ACK_16      0x22F
#define DSP_VERSION     0xE1

#define DSP_PROG_16     0xB0
#define DSP_PROG_8      0xC0
#define DSP_AUTO_INIT   0x06
#define DSP_PLAY        0x00
#define DSP_RECORD      0x08
#define DSP_MONO        0x00
#define DSP_STEREO      0x20
#define DSP_UNSIGNED    0x00
#define DSP_SIGNED      0x10

#define DMA_CHANNEL_16  5
#define DMA_FLIP_FLOP   0xD8
#define DMA_BASE_ADDR   0xC4
#define DMA_COUNT       0xC6

#define DSP_SET_TIME    0x40
#define DSP_SET_RATE    0x41
#define DSP_ON          0xD1
#define DSP_OFF         0xD3
#define DSP_OFF_8       0xD0
#define DSP_ON_8        0xD4
#define DSP_OFF_16      0xD5
#define DSP_ON_16       0xD6
#define DSP_VERSION     0xE1
char _soundbuf[4096];

void dsp_write(u8 b) {
    while (inportb(DSP_WRITE) & 0x80);
    outportb(DSP_WRITE, b);
}

void dsp_read(u8 b) {
    while (inportb(DSP_READ_STATUS) & 0x80);
    outportb(DSP_READ, b);
}

int dsp_reset()
{
    u8 major, minor;
    outportb(DSP_RESET, 1);

    // TODO: maybe not necessary
    // ~3 microseconds?
    for (size_t i = 0; i < 1000000; i++);

    outportb(DSP_RESET, 0);

    u8 status = inportb(DSP_READ_STATUS);
    if (~status & 128) {
        goto fail;
    }

    status = inportb(DSP_READ);
    if (status != 0xAA) {
        goto fail;
    }

    outportb(DSP_WRITE, DSP_VERSION);
    major = inportb(DSP_READ), minor = inportb(DSP_READ);

    if (major < 4) {
        status = (major << 4) | minor;
        goto fail;
    }
    return 1;
fail:
    kprint("Could not reset SB16");
    return 0;
}

static void dsp_setup_dma(uint32_t* buf1, uint32_t len)
{
    u8 mode = 0x48;

    // disable DMA channel
    outportb(DSP_ON_8, 4 + (DMA_CHANNEL_16 % 4));

    outportb(DMA_FLIP_FLOP, 1);
    outportb(DSP_ON_16, (DMA_CHANNEL_16 % 4) | mode | (1 << 4));

    u16 offset = (((uint64_t) buf1) / 2) % 65536;
    outportb(DMA_BASE_ADDR, (u8) ((offset >> 0) & 0xFF));
    outportb(DMA_BASE_ADDR, (u8) ((offset >> 8) & 0xFF));

    outportb(DMA_COUNT, (u8) (((len - 1) >> 0) & 0xFF));
    outportb(DMA_COUNT, (u8) (((len - 1) >> 8) & 0xFF));
    outportb(0x8B, ((uint64_t) buf1) >> 16);
    outportb(0xD4, DMA_CHANNEL_16 % 4);
}

static void dsp_setsample(u16 hz)
{
    dsp_write(DSP_SET_RATE);
    dsp_write((u8)((hz >> 8) & 0xFF));
    dsp_write((u8)(hz & 0xFF));
}

struct vfile* dsp_map()
{
    dsp_setup_dma((uint32_t*)_soundbuf, sizeof(_soundbuf));
    struct vfile* vf = (struct vfile*)kalloc(sizeof(struct vfile*), KERN_MEM);
    vf->mem = _soundbuf;
    vf->size = sizeof(_soundbuf);
    vf->name = "/home/dev/sb16";
    return vf;
}

void dsp_setup_file()
{
    dsp_setup_dma((uint32_t*)_soundbuf, sizeof(_soundbuf));
    sysvf[10].mem = _soundbuf;
    sysvf[10].size = sizeof(_soundbuf);
    sysvf[10].name = strdup("/home/dev/sb16");   
}

int sb16_enabled = 0;
void dsp_init()
{
    if(dsp_reset())
        sb16_enabled = 1; // no SB16 installed
    else return;
    dsp_setup_dma((uint32_t*)_soundbuf, sizeof(_soundbuf));
    dsp_setsample(44100);
    u16 sampleCount = (sizeof(_soundbuf) / 2) - 1;
    dsp_write(DSP_PLAY | DSP_PROG_16 | DSP_AUTO_INIT);
    dsp_write(DSP_SIGNED | DSP_MONO);
    dsp_write((u8) ((sampleCount >> 0) & 0xFF));
    dsp_write((u8) ((sampleCount >> 8) & 0xFF));

    dsp_write(DSP_ON);
    dsp_write(DSP_ON_16);    
    dsp_setup_file();
}

int getsoundinfo(){
 int u = inb(0x200);
 return u;
}
