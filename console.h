#pragma once
#include "tty.c"
#include "lib.c"
#include "screen.h"
#include "gui.c"
#include "drivers/fb.h"
#define C_CNT 16
#define CONSOLE_TTY 1
#define CONSOLE_GRAPHIC 2
#define CONSOLE_BASIC 4
#define CONSOLE_FILE 8 // just logs to a file

struct console 
{
    int w, h;
    int x, y; // cursor position
    char crs[C_CNT];
    int fd, flags;
    struct fb_font font;
} dbg_console;

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

struct window* kshow_console(char* name, struct console* console)
{
    struct window* win;
    if(console->flags & CONSOLE_GRAPHIC)
    {
        win = wmout.create(name, 0, 0, console->w, console->h);
        wmout.show(win);
    }
    return win;
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

struct console* file_console(char* filename)
{
    int fd = user_fsopen(filename, F_READ);
    struct console* cns = (struct console*)kalloc(sizeof(struct console), KERN_MEM);
    cns->fd = fd;
    cns->x = cns->y = 0;
    struct stat stat;
    ext2_stat(filename, &stat, fs_dev, fs_dev->priv);
    cns->w = stat.st_size;
    cns->h = 1;
    return cns;
}

struct console* alloc_console()
{
    struct console* cons = knew_console();
    kattach_console(cons, myproc());
    return cons;
}

void kdisp_text(struct console* cns, char* msg)
{
    struct fb_rect rect;
    rect.x1 = cns->x;
    rect.y1 = cns->y;
    rect.x2 = cns->x + cns->font.width;
    rect.y2 = cns->y + cns->font.height;
    while(*msg)
    {
        // kspec_char() - supports only text mode
        fb_write_char(font, *msg++, rect);
        rect.x1 = rect.x2 + 1;
        rect.x2 += cns->font.width;
        if(rect.x1 > fb_info.res_x)
        {
            rect.x1 = 0;
            rect.y1 += cns->font.height + 1;
        }
    }
}

void klog(struct console* cns, char* msg)
{
    if(cns->flags & CONSOLE_TTY) {
        ttywrite(cns->fd - TTYBASE, msg);
    } else if(cns->flags & CONSOLE_GRAPHIC) {
        kdisp_text(cns, msg);
    } else {
        movecr(cns->x, cns->y); // move the cursor to the position
        kprint(msg); // standard vga text mode
    }
}

void kunput(struct console* cns)
{
    if(cns->flags & CONSOLE_BASIC) {
        if(cns->x == 0)
            cns->x = 25, cns->y--;
        else
            cns->x--;
        kersc();
    } else if(cns->flags & CONSOLE_GRAPHIC) {
        
    }
}

void kdebuglog(char* msg)
{
    if(dbg_console.fd == 0)
        dbg_console.fd = TTYBASE;
    if(ttys[0].getc == NULL_PTR)
    {
        ttys[0].getc = uartgetc;
        ttys[0].putc = uartputc;
    }
    klog(&dbg_console, msg);
}

void __console_write(struct vfile* vf, char* msg, size_t sz)
{
    struct console cns;
    cns.fd = vf->fd;
    cns.flags = vf->status;
    klog(&cns, msg);
}

// sets the stdout of the process to the console
void kattach_console(struct console* cons, struct proc* p)
{
    p->std[1].fd = cons->fd;
    p->std[1].size = cons->w * cons->h;
    p->std[1].mem = kalloc(p->std[1].size, KERN_MEM);
    if(p->std[1].ops == NULL_PTR)
        p->std[1].ops = kalloc(sizeof(struct vfile*), KERN_MEM);
    struct vfileops* vf = (struct vfileops*)p->std[1].ops;
    vf->write = __console_write;
}