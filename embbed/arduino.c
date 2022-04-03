#pragma once
#define __ARDUINO__ 1
#include "../port.h"
#include "avr.h"
#define SREG _SFR_IO8(0x3F)
#define DDRB _SFR_IO8(0x04)
#define PINB _SFR_IO8(0x03)
#define PINC _SFR_IO8(0x06)
#define PIND _SFR_IO8(0x09)
#define SERIAL  0x0
#define DISPLAY 0x1
#define NOT_A_PIN -1
#define NOT_A_PORT -1
#define interrupts() sei()
#define noInterrupts() cli()

#define F_CPU 1000000UL
#define clockCyclesPerMicrosecond() ( F_CPU / 1000000L )
#define clockCyclesToMicroseconds(a) ( (a) / clockCyclesPerMicrosecond() )
#define microsecondsToClockCycles(a) ( (a) * clockCyclesPerMicrosecond() )

#define PROGMEM
#define __LPM_enhanced__(addr)  \
(__extension__({                \
    uint16_t __addr16 = (uint16_t)(addr); \
    uint8_t __result;           \
    asm volatile        \
    (                           \
        "lpm %0, Z" "\n\t"      \
        : "=r" (__result)       \
        : "z" (__addr16)        \
    );                          \
    __result;                   \
}))

#define __LPM_word_enhanced__(addr)         \
(__extension__({                            \
    uint16_t __addr16 = (uint16_t)(addr);   \
    uint16_t __result;                      \
    __asm__ __volatile__                    \
    (                                       \
        "lpm %A0, Z+"   "\n\t"              \
        "lpm %B0, Z"    "\n\t"              \
        : "=r" (__result), "=z" (__addr16)  \
        : "1" (__addr16)                    \
    );                                      \
    __result;                               \
}))

const uint16_t port_to_input_PGM[] = {
        NOT_A_PORT,
        NOT_A_PORT,
        (uint16_t) &PINB,
        (uint16_t) &PINC,
        (uint16_t) &PIND,
};

#define pgm_read_word(address_short) __LPM_word_enhanced__((uint16_t)(address_short))
#define portInputRegister(P) ( (volatile uint8_t *)( pgm_read_word( port_to_input_PGM + (P))) )

#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define __LPM(x) __LPM_enhanced(x)
#define HIGH (1)
#define LOW (0)

#define bitRead(value, bit)  (((value) >> (bit)) & 0x01)
#define bitSet(value, bit)   ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) ((bitvalue) ? bitSet(value, bit) : bitClear(value, bit))

#define pgm_read_byte(address_short) __LPM((uint16_t)(address_short))
const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[] = {
        _BV(0), /* 0, port D */ 
        _BV(1),
        _BV(2),
        _BV(3),
        _BV(4),
        _BV(5),
        _BV(6),
        _BV(7),
        _BV(0), /* 8, port B */
        _BV(1),
        _BV(2),
        _BV(3),
        _BV(4),
        _BV(5),
        _BV(0), /* 14, port C */
        _BV(1),
        _BV(2),
        _BV(3),
        _BV(4),
        _BV(5),
};

const uint8_t PROGMEM digital_pin_to_port_PGM[] = {
        PD, /* 0 */
        PD,
        PD,
        PD,
        PD,
        PD,
        PD,
        PD,
        PB, /* 8 */
        PB,
        PB,
        PB,
        PB,
        PB,
        PC, /* 14 */
        PC,
        PC,
        PC,
        PC,
        PC,
};

const uint8_t PROGMEM digital_pin_to_timer_PGM[] = {
	NOT_ON_TIMER, /* 0 - port D */
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	TIMER2B,
	NOT_ON_TIMER,
	TIMER0B,
	TIMER0A,
	NOT_ON_TIMER,
	NOT_ON_TIMER, /* 8 - port B */
	TIMER1A,
	TIMER1B,
	TIMER2A,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER, /* 14 - port C */
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
};
#define NOT_ON_TIMER (0)
#define digitalPinToBitMask(P) ( pgm_read_byte( digital_pin_to_bit_mask_PGM + (P) ) )
#define digitalPinToPort(P) ( pgm_read_byte( digital_pin_to_port_PGM + (P) ) )
#define digitalPinToTimer(P) ( pgm_read_byte( digital_pin_to_timer_PGM + (P) ) )

static void turnOffPWM(uint8_t timer)
{
    switch (timer)
    {
        case  TIMER1A:  cbi(TCCR1A, COM1A1);    break;
        case  TIMER1B:  cbi(TCCR1A, COM1B1);    break;
        case  TIMER0A:  cbi(TCCR0A, COM0A1);    break;
        case  TIMER0B:  cbi(TCCR0A, COM0B1);    break;
        case  TIMER2A:  cbi(TCCR2A, COM2A1);    break;
        case  TIMER2B:  cbi(TCCR2A, COM2B1);    break;
    }
}

void pinMode(uint8_t pin, uint8_t mode)
{
        uint8_t bit = digitalPinToBitMask(pin);
        uint8_t port = digitalPinToPort(pin);
        volatile uint8_t *reg, *out;

        if (port == NOT_A_PIN) 
           return;

        reg = portModeRegister(port);
        out = portOutputRegister(port);

        if (mode == INPUT) {
                uint8_t oldSREG = SREG;
                cli();
                *reg &= ~bit;
                *out &= ~bit;
                SREG = oldSREG;
        } else if (mode == INPUT_PULLUP) {
                uint8_t oldSREG = SREG;
                cli();
                *reg &= ~bit;
                *out |= bit;
                SREG = oldSREG;
        } else {
                uint8_t oldSREG = SREG;
                cli();
                *reg |= bit;
                SREG = oldSREG;
        }
}

int digitalRead(uint8_t pin)
{
        uint8_t timer = digitalPinToTimer(pin);
        uint8_t bit = digitalPinToBitMask(pin);
        uint8_t port = digitalPinToPort(pin);

        if (port == NOT_A_PIN) 
          return LOW;

        if (timer != NOT_ON_TIMER) turnOffPWM(timer);

        if (*portInputRegister(port) & bit) return HIGH;
        return LOW;
}