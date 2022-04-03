#pragma once
#include "port.h"
#define SECS    0x00
#define MINS    0x02
#define HOURS   0x04
#define DAY     0x07
#define MONTH   0x08
#define YEAR    0x09
#define CMOS_PORT 0x70
#define CMOS_RETURN  0x71   
#define TIMER_FD 0xFF0
#define rdtscl(dest) \
   asm("mfc0 %0,$9; nop" : "=r" (dest));
#define QNOP() asm("nop; nop; nop; nop")

struct rtcdate {
  int second;
  int minute;
  int hour;
  int day;
  int month;
  int year;
};

uint64_t readtsc(void) {
	uint32_t lo, hi;
	asm volatile("rdtsc" : "=a"(lo), "=d"(hi));
	return ((uint64_t)hi << 32) | (uint64_t)lo;
}

void delay(int u){
  int a;
  for(a=0;a < u;a++){}
}

void microdelay(int us){}

static int cmos_read(int reg)
{
  outb(CMOS_PORT,  reg);
  microdelay(200);
  return inb(CMOS_RETURN);
}

static void filldate(struct rtcdate *r)
{
  r->second = cmos_read(SECS);
  r->minute = cmos_read(MINS);
  r->hour   = cmos_read(HOURS);
  r->day    = cmos_read(DAY);
  r->month  = cmos_read(MONTH);
  r->year   = cmos_read(YEAR);
}

// to remove the 32-bit CMOS limitation
void fixdate(struct rtcdate *r) {
  if (r->year == 1901 && r->month == 0){
	  r->year = 2038;
  }
}

void rwait(int seconds) {
	size_t secs = cmos_read(SECS);
	size_t current = cmos_read(SECS);
	while (current - secs < seconds) {
	   current = cmos_read(SECS);
	   delay(10000);
	}
}

int timefunc(void (*x)())
{
  int x = cmos_read(SECS);
  x();
  int y = cmos_read(SECS);
  return (y - x);
}

void waittick(){
  asm("sti; hlt");
}

#define HPETSECS(num) ((num)*1000000)
#define HPETMSECS(num) ((num)*10000)
#define HPETMICSECS(num) ((num)*10)

#define HPET2SECS(num) ((num) / 1000000)
#define HPET2MSECS(num) ((num) / 10000)
#define HPET2MICSECS(num) ((num) / 10)
size_t clk = 0;

struct HPET
{
  uint64_t general_capabilities;
  uint64_t reserved;
  uint64_t general_configuration;
  uint64_t reserved2;
  uint64_t general_status;
  uint64_t reserved3;
  uint64_t reserved4[24];
  uint64_t main_counter_value;
  uint64_t reserved5;
} *hpet;

uint64_t hpetcount()
{
  return mminq(&hpet->main_counter_value);
}

void hpet_init()
{
  clk = hpet->general_capabilities >> 32;
}