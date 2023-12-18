#pragma once
#define SB16_CARD 2
#define INTEL_CARD 4
#define REALTEK_CARD 16
#define OTHER_CARD 256
#include "../port.h"
#include "../speaker.h"

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

void dsp_write(u8 b) {
    while (inportb(DSP_WRITE) & 0x80);
    outportb(DSP_WRITE, b);
}

void dsp_read(u8 b) {
    while (inportb(DSP_READ_STATUS) & 0x80);
    outportb(DSP_READ, b);
}

void dsp_reset()
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

fail:
    kprint("Could not reset SB16");
    return;
}

int getsoundinfo(){
 int u = inb(0x200);
 return u;
}

void testsound(){
 
}
