#pragma once
#include "port.h"
#include "lib.c"
#define LOW_BYTE(x)    (x & 0x00FF)
#define HI_BYTE(x)     ((x & 0xFF00) >> 8)

typedef struct
{
    char page;
    size_t offset;
    size_t length;
} dmablock;

uint8_t MaskReg[8]   = { 0x0A, 0x0A, 0x0A, 0x0A, 0xD4, 0xD4, 0xD4, 0xD4 };
uint8_t ModeReg[8]   = { 0x0B, 0x0B, 0x0B, 0x0B, 0xD6, 0xD6, 0xD6, 0xD6 };
uint8_t ClearReg[8]  = { 0x0C, 0x0C, 0x0C, 0x0C, 0xD8, 0xD8, 0xD8, 0xD8 };

uint8_t PagePort[8]  = { 0x87, 0x83, 0x81, 0x82, 0x8F, 0x8B, 0x89, 0x8A };
uint8_t AddrPort[8]  = { 0x00, 0x02, 0x04, 0x06, 0xC0, 0xC4, 0xC8, 0xCC };
uint8_t CountPort[8] = { 0x01, 0x03, 0x05, 0x07, 0xC2, 0xC6, 0xCA, 0xCE };

void dmastart(uint8_t channel, dmablock* blk, uint8_t mode)
{
    mode |= channel;
    CLI(); //disable interrupts
    outb(MaskReg[channel], 0x04 | channel);
    outb(ClearReg[channel], 0x00);
    outportb(ModeReg[channel], mode);

    outportb(AddrPort[channel], LOW_BYTE(blk->offset));
    outportb(AddrPort[channel], HI_BYTE(blk->offset));
    outportb(PagePort[channel], blk->page);

    outportb(CountPort[channel], LOW_BYTE(blk->length));
    outportb(CountPort[channel], HI_BYTE(blk->length));

    outportb(MaskReg[channel], channel);
    STI();
}

void dmapause(uint8_t channel)
{
    outportb(MaskReg[channel], 0x04 | channel);
}

void dmacont(uint8_t channel)
{
    outportb(MaskReg[channel], channel);
}