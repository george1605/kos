#pragma once
#include "irq.c"
#include "lib.c"
#include "time.c"
#include "vfs.h"
#define LPT1 0x3BC
#define LPT2 0x378
#define LPT3 0x278

struct pardev
{
    struct spinlock lock;
    int* buffer;
    int type;
};

void parlock(struct pardev i)
{
    acquire(&i.lock);
}

void parunlock(struct pardev i)
{
    release(&i.lock);
}

void init_parallel(uint16_t port)
{
    outportb(port + 2, 0x0C);                      // Activates SLCT INP
    outportb(port + 2, inportb(port + 2) & 0x0FB); // Send the INIT Signal
    outportb(port + 2, inportb(port + 2) | 0x04);
}

size_t read_parallel(uint16_t port)
{
    size_t data;
    data = inportb(port + 1);
    return data;
}

void write_parallel(uint16_t port, uint8_t data)
{
    while (!inportb(port + 1) & 0x80)
    {
        NOP();
    }
    outportb(port + 0, data);                      // Places the data on the data lines
    outportb(port + 2, inportb(port + 2) | 0x01);  // Activate STROBE
    outportb(port + 2, inportb(port + 2) & 0x0FE); // Everything has an end, deactivate STROBE
}

void parwrite(struct pardev i, char *u)
{
    int n = 0;
    while (u[n] != 0)
    {
        write_parallel(i.type, (uint8_t)u[n]);
        n++;
    }
}

struct vfile parmap(char *fname, struct pardev i)
{
    struct vfile u;
    u.fd = 0x1C0000 + i.type;
    u.status = 0x8;
    u.mem = NULL_PTR;
    u.name = fname;
    return u;
}

char *parread(struct pardev u, size_t size)
{
    if (u.buffer == 0)
        u.buffer = alloc(0, 1024);

    int i = 0;
    while (i < size)
    {
        u.buffer[i] = read_parallel(u.type);
        i++;
    }
    return (char*)u.buffer;
}

void parfree(struct pardev u)
{
    parunlock(u);
    if (u.buffer != 0)
        free(u.buffer);
}

void par_handler(struct regs* r)
{
    read_parallel(LPT1);
}

void par_init()
{
    irq_install_handler(7,par_handler);
}