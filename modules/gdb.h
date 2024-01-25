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

void gdb_setflags(struct proc* p, size_t flags)
{
    flags &= ~(1 << 12); // clear the iopl so that it doesn't get io perms
    flags &= ~(1 << 13); 
    pusheflags(flags);
}

uint8_t gdb_getxmm(struct proc* p, int nr, uint64_t* low, uint64_t* high)
{
    if(nr > 15) return -1;
    uint16_t offset = 0xA0 + (0x10 * nr);
    *low = *(uint64_t*)((char*)p->ctx + offset); // Bro, could do memcpy
    *high = *(uint64_t*)((char*)p->ctx + offset + 8);
}

void gdb_setxmm(struct proc* p, int nr, uint64_t low, uint64_t high)
{
    uint16_t offset = 0xA0 + (0x10 * nr);
    char* ptr = (char*)p->ctx + offset;
    *(uint64_t*)ptr = high;
    *(uint64_t*)(ptr + 8) = low;
}