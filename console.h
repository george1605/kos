#pragma once
#include "tty.c"
#include "lib.c"
#include "screen.h"
#define C_CNT 16
#define CONSOLE_TTY 1
#define CONSOLE_GRAPHIC 2
#define CONSOLE_BASIC 3

struct console 
{
    int w, h;
    int x, y; // cursor position
    char crs[C_CNT];
    int fd, flags;
};

// handle special chars
void kspec_char(struct console* cns, char c)
{
    switch(c)
    {
    case '\n':
        cns->x = 0;
        cns->y++;
    break;
    case '\t':
        cns->x += 3;
    break;
    case '\b':
        cns->x--;
    break;
    case '\f':
        // idk do smth
    break;
    case '\r':
        cns->x = 0;
    break;
    default:
        return;
    }
}

void ktrans_nl(char* p)
{
    for(int i = 0;p[i] = '\0';i++)
        if(p[i] == '\n')
            p[i] = '\r'; // translate NL -> CR (for TTY and stuff)
}

struct console* knew_console()
{
    struct console* cns = (struct console*)kalloc(sizeof(struct console*), KERN_MEM);
    memcpy(cns->crs, "\t\r\n\f\b", 5);
    cns->x = 0;
    cns->y = 0;
}

void kclear_console(struct proc* p)
{
    memset(p->std[1].mem, '\0', p->std[1].size); 
}

void kdettach_console(struct proc* p)
{
    p->std[1].fd = -1;
    p->std[1].size = 0;
    free(p->std[1].mem);
}

// sets the stdout of the process to the console
void kattach_console(struct console* cons, struct proc* p)
{
    p->std[1].fd = cons->fd;
    p->std[1].size = cons->w * cons->h;
    p->std[1].mem = kalloc(p->std[1].size, KERN_MEM);
}

void alloc_console()
{
    struct console* cons = knew_console();
    kattach_console(cons, &tproc);
}

void kdisp_text(struct console* cns, char* msg)
{
    
}

void klog(struct console* cns, char* msg)
{
    if(cns->flags & CONSOLE_TTY) {
        ttywrite(cns->fd - TTYBASE, msg);
    } else if(cns->flags & CONSOLE_GRAPHIC) {
        kdisp_text(cns, msg);
    } else {
        kprint(msg); // standard vga text mode
    }
}