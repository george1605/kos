#include "gcc.h"
#define NULL_STACK -1
#define INVALID_PID -2
int gdb_err = 0;

void* __get_stack(int pid)
{
    struct proc u = procid(pid);
    if(u.stack == NULL_PTR)
        gdb_err = NULL_STACK;
    return u.stack;
}

int __get_pflags(int pid)
{
    struct proc u = procid(pid);
    return (int)u.state;
}