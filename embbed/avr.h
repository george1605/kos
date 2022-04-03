#pragma once
#include "../lib.c"
#define __AVR__ 1
typedef int  int_farptr_t;
typedef size_t uint_farptr_t;

#define AVRIO_PIN(avrport, avrbit) ((avrport << 4) | avrbit)
#define cli() CLI()
#define _VECTOR(n) __vector_ ## n
#define sei() asm volatile("sei" ::)
#define RAM_LIMIT  64 * 1024
#define RAM_LIMIT2 4 * RAM_LIMIT
#define RAM_LIMIT3 1024 * 1024
#define __COMMON_ASRE 4
#define PGERS   1
#define PGWRT   2
void abort(void)
{
  for (;;);// or while(true)
}

#define SREG_C  0
#define SREG_Z  1
#define SREG_N  2
#define SREG_V  3
#define SREG_S  4
#define SREG_H  5
#define SREG_T  6
#define SREG_I  7

#define _SFR_MEM_ADDR(sfr)   ((uint16_t) (&sfr))
#define SELFPRGEN   0
#define __SPM_ENABLE SELFPRGEN
#define GET_LOW_FUSE_BITS   0x0000
#define GET_LOCK_BITS   0x0001
#define GET_EXTENDED_FUSE_BITS   0x0002
#define _MMIO(type,addr) (*(volatile unsigned type*)addr)
#define __SFR_OFFSET 0x20
#define _MMIO_BYTE(mem_addr) _MMIO(char,mem_addr)
#define _MMIO_WORD(mem_addr) _MMIO(short,mem_addr)
#define _SFR_IO8(io_addr) _MMIO_BYTE((io_addr) + __SFR_OFFSET)
#define _SFR_IO16(io_addr) _MMIO_WORD((io_addr) + __SFR_OFFSET)
#define _SFR_BYTE(sfr) _MMIO_BYTE(_SFR_ADDR(sfr))
#define __SPM_REG _SFR_IO8(0x37)

#define BOOTLOADER_SECTION __attribute__ ((section ('.bootloader'))) 
#define _BV(bit) (1 << bit) /* leftshiftting */
#define _BX(a,b) (a ^ b)
#define _SFR_MEM8(mem_addr) _MMIO_BYTE(mem_addr)
#define _SFR_MEM16(mem_addr) _MMIO_WORD(mem_addr)
#define ADMUX _SFR_MEM8(0x7C)

#define __BOOT_PAGE_ERASE (_BV(__SPM_ENABLE) | _BV(PGERS))
#define __BOOT_PAGE_WRITE (_BV(__SPM_ENABLE) | _BV(PGWRT))
#define __BOOT_PAGE_FILL _BV(__SPM_ENABLE) 
#define __BOOT_RWW_ENABLE (_BV(__SPM_ENABLE) | _BV(__COMMON_ASRE)) 

#define PORTB _SFR_IO8(0x05)
#define PORTC _SFR_IO8(0x08)
#define PORTD _SFR_IO8(0x0B)

#define ADCSRA _SFR_MEM8(0x7A)
#define ADPS0 0
#define ADPS1 1
#define ADPS2 2
#define ADIE 3
#define ADIF 4
#define ADATE 5
#define ADSC 6
#define ADEN 7

 #define sleep_cpu()                              \
  do {                                             \
     asm volatile ( "sleep" "\n\t" :: );    \
  } while(0)
   
#define __boot_page_fill_normal(address, data)   \
(__extension__({                                 \
    asm volatile                         \
    (                                            \
        "movw  r0, %3\n\t"                       \
        "sts %0, %1\n\t"                         \
        "spm\n\t"                                \
        "clr  r1\n\t"                            \
        :                                        \
        : "i" (0x20 + 0x37),        \
          "r" ((uint8_t)(__BOOT_PAGE_FILL)),     \
          "z" ((uint16_t)(address)),             \
          "r" ((uint16_t)(data))                 \
        : "r0"                                   \
    );                                           \
}))

#define __boot_page_fill_alternate(address, data)\
(__extension__({                                 \
    asm volatile                                 \
    (                                            \
        "movw  r0, %3\n\t"                       \
        "sts %0, %1\n\t"                         \
        "spm\n\t"                                \
        ".word 0xffff\n\t"                       \
        "nop\n\t"                                \
        "clr  r1\n\t"                            \
        :                                        \
        : "i" (0x37 + 0x20),        \
          "r" ((uint8_t)(__BOOT_PAGE_FILL)),     \
          "z" ((uint16_t)(address)),             \
          "r" ((uint16_t)(data))                 \
        : "r0"                                   \
    );                                           \
}))

#define __boot_page_fill_extended(address, data) \
(__extension__({                                 \
    asm volatile                                 \
    (                                            \
        "movw  r0, %4\n\t"                       \
        "movw r30, %A3\n\t"                      \
        "sts %1, %C3\n\t"                        \
        "sts %0, %2\n\t"                         \
        "spm\n\t"                                \
        "clr  r1\n\t"                            \
        :                                        \
        : "i" (0x30 + 0x27),        \
          "i" (_SFR_MEM_ADDR(RAMPZ)),            \
          "r" ((uint8_t)(__BOOT_PAGE_FILL)),     \
          "r" ((uint32_t)(address)),             \
          "r" ((uint16_t)(data))                 \
        : "r0", "r30", "r31"                     \
    );                                           \
}))

#define __boot_page_erase_normal(address)        \
(__extension__({                                 \
    asm volatile                                 \
    (                                            \
        "sts %0, %1\n\t"                         \
        "spm\n\t"                                \
        :                                        \
        : "i" (_SFR_MEM_ADDR(__SPM_REG)),        \
          "r" ((uint8_t)(__BOOT_PAGE_ERASE)),    \
          "z" ((uint16_t)(address))              \
    );                                           \
}))

#define __boot_page_erase_alternate(address)     \
(__extension__({                                 \
    asm volatile                        \
    (                                            \
        "sts %0, %1\n\t"                         \
        "spm\n\t"                                \
        ".word 0xffff\n\t"                       \
        "nop\n\t"                                \
        :                                        \
        : "i" (_SFR_MEM_ADDR(__SPM_REG)),        \
          "r" ((uint8_t)(__BOOT_PAGE_ERASE)),    \
          "z" ((uint16_t)(address))              \
    );                                           \
}))

#define __boot_page_erase_extended(address)      \
(__extension__({                                 \
    asm volatile                         \
    (                                            \
        "movw r30, %A3\n\t"                      \
        "sts  %1, %C3\n\t"                       \
        "sts %0, %2\n\t"                         \
        "spm\n\t"                                \
        :                                        \
        : "i" (_SFR_MEM_ADDR(__SPM_REG)),        \
          "i" (_SFR_MEM_ADDR(RAMPZ)),            \
          "r" ((uint8_t)(__BOOT_PAGE_ERASE)),    \
          "r" ((uint32_t)(address))              \
        : "r30", "r31"                           \
    );                                           \
}))

#define __boot_page_write_normal(address)        \
(__extension__({                                 \
    asm volatile                         \
    (                                            \
        "sts %0, %1\n\t"                         \
        "spm\n\t"                                \
        :                                        \
        : "i" (_SFR_MEM_ADDR(__SPM_REG)),        \
          "r" ((uint8_t)(__BOOT_PAGE_WRITE)),    \
          "z" ((uint16_t)(address))              \
    );                                           \
}))

#define __boot_page_write_alternate(address)     \
(__extension__({                                 \
    asm volatile                         \
    (                                            \
        "sts %0, %1\n\t"                         \
        "spm\n\t"                                \
        ".word 0xffff\n\t"                       \
        "nop\n\t"                                \
        :                                        \
        : "i" (_SFR_MEM_ADDR(__SPM_REG)),        \
          "r" ((uint8_t)(__BOOT_PAGE_WRITE)),    \
          "z" ((uint16_t)(address))              \
    );                                           \
}))

#define __boot_page_write_extended(address)      \
(__extension__({                                 \
    asm volatile                         \
    (                                            \
        "movw r30, %A3\n\t"                      \
        "sts %1, %C3\n\t"                        \
        "sts %0, %2\n\t"                         \
        "spm\n\t"                                \
        :                                        \
        : "i" (_SFR_MEM_ADDR(__SPM_REG)),        \
          "i" (_SFR_MEM_ADDR(RAMPZ)),            \
          "r" ((uint8_t)(__BOOT_PAGE_WRITE)),    \
          "r" ((uint32_t)(address))              \
        : "r30", "r31"                           \
    );                                           \
}))

#define __boot_rww_enable()                      \
(__extension__({                                 \
   asm volatile                        \
    (                                            \
        "sts %0, %1\n\t"                         \
        "spm\n\t"                                \
        :                                        \
        : "i" (_SFR_MEM_ADDR(__SPM_REG)),        \
          "r" ((uint8_t)(__BOOT_RWW_ENABLE))     \
    );                                           \
}))

#define __boot_rww_enable_alternate()            \
(__extension__({                                 \
    asm volatile                        \
    (                                            \
        "sts %0, %1\n\t"                         \
        "spm\n\t"                                \
        ".word 0xffff\n\t"                       \
        "nop\n\t"                                \
        :                                        \
        : "i" (_SFR_MEM_ADDR(__SPM_REG)),        \
          "r" ((uint8_t)(__BOOT_RWW_ENABLE))     \
    );                                           \
}))

#define __boot_lock_bits_set(lock_bits)                    \
(__extension__({                                           \
    uint8_t value = (uint8_t)(~(lock_bits));               \
    asm volatile                                   \
    (                                                      \
        "ldi r30, 1\n\t"                                   \
        "ldi r31, 0\n\t"                                   \
        "mov r0, %2\n\t"                                   \
        "sts %0, %1\n\t"                                   \
        "spm\n\t"                                          \
        :                                                  \
        : "i" (_SFR_MEM_ADDR(__SPM_REG)),                  \
          "r" ((uint8_t)(__BOOT_LOCK_BITS_SET)),           \
          "r" (value)                                      \
        : "r0", "r30", "r31"                               \
    );                                                     \
}))

#define __boot_lock_bits_set_alternate(lock_bits)          \
(__extension__({                                           \
    uint8_t value = (uint8_t)(~(lock_bits));               \
    asm volatile                                   \
    (                                                      \
        "ldi r30, 1\n\t"                                   \
        "ldi r31, 0\n\t"                                   \
        "mov r0, %2\n\t"                                   \
        "sts %0, %1\n\t"                                   \
        "spm\n\t"                                          \
        ".word 0xffff\n\t"                                 \
        "nop\n\t"                                          \
        :                                                  \
        : "i" (_SFR_MEM_ADDR(__SPM_REG)),                  \
          "r" ((uint8_t)(__BOOT_LOCK_BITS_SET)),           \
          "r" (value)                                      \
        : "r0", "r30", "r31"                               \
    );                                                     \
}))

void __main_start();
void __boot_start(){
  void* u = PORTB;
  __boot_rww_enable();
  __boot_page_write_normal(u);
}
