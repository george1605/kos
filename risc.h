#pragma once
#include "lib.c"
#define SBI_SET_TIMER 0x0
#define SBI_CONSOLE_PUTCHAR 0x1
#define SBI_CONSOLE_GETCHAR 0x2
#define SBI_SHUTDOWN 0x8
#define RMOVE(reg1, reg2) \
    asm("mv " #reg1 ", " #reg2);
#define SCALL() asm("scall")
#define SBREAK() asm("sbreak")
#define RA asm("ra")
#define SP asm("sp")
typedef unsigned long __ulong;

struct riscargs
{
    size_t ret;
    size_t fid;
    size_t error;
    size_t value;
};

// moves the arguments in the registers and performs the call
struct riscargs sbicall(__ulong arg0, __ulong arg1, __ulong arg2, __ulong arg3,
                        __ulong arg4, __ulong arg5, __ulong fid, __ulong ext)
{
    register size_t a0 asm("a0") = (size_t)arg0;
    register size_t a1 asm("a1") = (size_t)arg1;
    register size_t a2 asm("a2") = (size_t)arg2;
    register size_t a3 asm("a3") = (size_t)arg3;
    register size_t a4 asm("a4") = (size_t)arg4;
    register size_t a5 asm("a5") = (size_t)arg5;
    register size_t a6 asm("a6") = (size_t)fid;
    register size_t a7 asm("a7") = (size_t)ext;
    asm volatile("ecall"
                 : "+r"(a0), "+r"(a1)
                 : "r"(a2), "r"(a3), "r"(a4), "r"(a5), "r"(a6), "r"(a7)
                 : "memory");
    struct riscargs u;
    u.ret = ext;
    u.fid = fid;
    u.error = a0;
    u.value = a1;
    return u;
}

void sbiputc(char _Char)
{
    sbicall(SBI_CONSOLE_PUTCHAR, 0, _Char, 0, 0, 0, 0, 0);
}

int sbigetc(void)
{
    struct riscargs ret;
    ret = sbicall(SBI_CONSOLE_GETCHAR, 0, 0, 0, 0, 0, 0, 0);
    return ret.error;
}

void sbisleep(){
    sbi_ecall(SBI_SHUTDOWN, 0, 0, 0, 0, 0, 0, 0);
}

void sbiputs(char* _Str){
 
}