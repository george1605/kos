#pragma once
#include "vga.h"
#include "i965.c"
#include "main.c"
typedef int(*TestFunction)(void);

int test(TestFunction first, ...) {
    va_list args;
    va_start(args, first);

    TestFunction func = first;
    int index = 0;

    while (func != NULL_PTR) {
        int result = func();
        if (result == 0) {
            va_end(args);
            return index;
        }
        index++;
        func = va_arg(args, TestFunction);
    }

    va_end(args);
    return -1;
}

void fb_setup()
{
    int i = test(mesa_init, i965_setup, vga_setup, NULL_PTR); // try newer first and if not use vga. if that also fails, use text mode
    if(i == -1)
    {
        fb_info.mem = (void*)0xB8000;
        fb_info.res_x = 80;
        fb_info.res_y = 25;
        kprint("Could not setup video!\nUsing text mode.");
    }
}