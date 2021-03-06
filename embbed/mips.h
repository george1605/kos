#include "../noarch.h"
#undef NOP
#undef ASMV
#define REG0 asm("$0")
#define REG1 asm("$1")
#define REG2 asm("$2")
#define JUMP(addr) asm("j %0":"r"(addr))
#define JUMPR(reg) asm("jr "#reg)

unsigned long getsp()
{
    unsigned long sp;
    asm volatile("move %0, $sp"
                         : "=r"(sp));
    return sp;
}