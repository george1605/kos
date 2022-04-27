#include "gcc.h"
#include "../system.h"
#define NULL_STACK -1
#define INVALID_PID -2
#define P_PEEK 1
#define P_POKE 2
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

long __SYSCALL ptrace(int flag, int pid, size_t arg0, size_t arg1)
{
    void* p = __get_stack(pid);
    switch(flag)
    {
        case P_PEEK:
            return *(p + arg0);
        case P_POKE:
            *(p + arg0) = arg1;
            return 0L;
    }
    return 1L;
}