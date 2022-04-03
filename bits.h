#pragma once
#define INTSIZE 32
#define shiftr_bit(a,b) a >> b
#define shiftl_bit(a,b) a << b
#define set_bit(a,b) a |= (1 << b)
#define clear_bit(a,b) a &= ~(1 << b);
#define wake_up_bit(a,b) 0

#define swap16_bit(x) \
     ((((x) >> 8) & 0xff) | (((x) & 0xff) << 8))
