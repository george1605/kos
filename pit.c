#pragma once
#include "port.h"
#include "irq.c"
#include "lib.c"
#define TIMER_TPS 363
#define PIT_A 0x40
#define PIT_B 0x41
#define PIT_C 0x42
#define PIT_CONTROL 0x43

#define PIT_MASK 0xFF
#define PIT_SET 0x36

#define PIT_HZ 1193181
#define DIV_OF_FREQ(_f) (PIT_HZ / (_f))
#define FREQ_OF_DIV(_d) (PIT_HZ / (_d))
#define REAL_FREQ_OF_FREQ(_f) (FREQ_OF_DIV(DIV_OF_FREQ((_f))))

size_t timer_ticks = 0;
int timer_active = 1;
int timer_secs = 0;

struct {
  long frequency;
  long divisor;
} timstate;

struct timer {
  size_t ticks;
  size_t expire;
  size_t elapse;
  int active;
  void(*f)(void);
};

struct timer timer_creat(){
  struct timer u;
  u.ticks = timer_ticks;
  u.active = 1;
  return u;
}

void timer_check(struct timer u){
  if(!u.active) return; 
  if(u.expire == u.elapse){
    u.f();
    u.elapse = 0;
  }
  u.elapse += (timer_ticks - u.ticks);
  u.ticks = timer_ticks;
}

void timer_kill(struct timer u){
  u.active = 0;
} 

static void timer_set(int hz) {
    outportb(PIT_CONTROL, PIT_SET);
    uint16_t d = (uint16_t) (1193131.666 / hz);
    outportb(PIT_A, d & PIT_MASK);
    outportb(PIT_A, (d >> 8) & PIT_MASK);
}

void timer_phase(int hz)
{
    int divisor = 1193180 / hz;       /* Calculate our divisor */
    outportb(0x43, 0x36);             /* Set our command byte 0x36 */
    outportb(0x40, divisor & 0xFF);   /* Set low byte of divisor */
    outportb(0x40, divisor >> 8);     /* Set high byte of divisor */
}

void timer_handler(struct regs *r)
{
    timer_ticks++;
}

void timer_install()
{
  irq_install_handler(0, timer_handler);
}

void timer_init() {
    const long freq = REAL_FREQ_OF_FREQ(TIMER_TPS);
    timstate.frequency = freq;
    timstate.divisor = DIV_OF_FREQ(freq);
    timer_ticks = 0;

    timer_set(TIMER_TPS);
    timer_install();
}

void picoff(){
   irq_change(4,0);
}