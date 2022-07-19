#pragma once
#include "../noarch.h"
#define MMIO32(x) (*(volatile unsigned int *)x)
#define PPBI_BASE 0xE0000000U
#define PERIPH_BASE 0x40000000U
#define FLASH_BASE 0x08000000U
#define PERIPH_BASE_APB1 PERIPH_BASE
#define PERIPH_BASE_AHB1 PERIPH_BASE + 0x00020000
#define SCS_BASE PPBI_BASE + 0xE000
#define MPU_BASE SCS_BASE + 0x0D90
#define RTC_BASE PERIPH_BASE_APB1 + 0x2800
#define FLASH_MEM_INTERFACE_BASE PERIPH_BASE_AHB1 + 0x2000
#define POWER_CONTROL_BASE PERIPH_BASE_APB1 + 0x7000
#define CTL_BASE MMIO32(PERIPH_BASE)

#define MPU_TYPE MMIO32(MPU_BASE + 0x00)
#define MPU_CTRL MMIO32(MPU_BASE + 0x04)
#define MPU_RNR MMIO32(MPU_BASE + 0x08)
#define MPU_RBAR MMIO32(MPU_BASE + 0x0C)
#define MPU_RASR MMIO32(MPU_BASE + 0x10)

#define RTC_TR MMIO32(RTC_BASE + 0x00)
#define RTC_DR MMIO32(RTC_BASE + 0x04)
#define RTC_CR MMIO32(RTC_BASE + 0x08)
#define RTC_ISR MMIO32(RTC_BASE + 0x0c)
#define RTC_PRER MMIO32(RTC_BASE + 0x10)
#define RTC_WUTR MMIO32(RTC_BASE + 0x14)
#define RTC_CALIBR MMIO32(RTC_BASE + 0x18)
#define RTC_ALRMAR MMIO32(RTC_BASE + 0x1c)
#define RTC_ALRMBR MMIO32(RTC_BASE + 0x20)
#define RTC_WPR MMIO32(RTC_BASE + 0x24)
#define RTC_SSR MMIO32(RTC_BASE + 0x28)
#define RTC_SHIFTR MMIO32(RTC_BASE + 0x2c)
#define RTC_TSTR MMIO32(RTC_BASE + 0x30)
#define RTC_TSDR MMIO32(RTC_BASE + 0x34)
#define RTC_TSSSR MMIO32(RTC_BASE + 0x38)
#define RTC_CALR MMIO32(RTC_BASE + 0x3c)
#define RTC_TAFCR MMIO32(RTC_BASE + 0x40)
#define RTC_ALRMASSR MMIO32(RTC_BASE + 0x44)
#define RTC_ALRMBSSR MMIO32(RTC_BASE + 0x48)

#define GPIO_BRR(port) MMIO32((port) + 0x28)
#define GPIO_ASCR(port) MMIO32((port) + 0x2C)

#define FLASH_ACR MMIO32(FLASH_MEM_INTERFACE_BASE + 0x00)
#define FLASH_KEYR MMIO32(FLASH_MEM_INTERFACE_BASE + 0x04)
#define FLASH_OPTKEYR MMIO32(FLASH_MEM_INTERFACE_BASE + 0x08)
#define FLASH_SR MMIO32(FLASH_MEM_INTERFACE_BASE + 0x0C)
#define FLASH_CR MMIO32(FLASH_MEM_INTERFACE_BASE + 0x10)
#define FLASH_AR MMIO32(FLASH_MEM_INTERFACE_BASE + 0x14)
#define FLASH_OBR MMIO32(FLASH_MEM_INTERFACE_BASE + 0x1C)
#define FLASH_WRPR MMIO32(FLASH_MEM_INTERFACE_BASE + 0x20)
#define FLASH_KEYR2 MMIO32(FLASH_MEM_INTERFACE_BASE + 0x44)
#define FLASH_SR2 MMIO32(FLASH_MEM_INTERFACE_BASE + 0x4C)
#define FLASH_CR2 MMIO32(FLASH_MEM_INTERFACE_BASE + 0x50)
#define FLASH_AR2 MMIO32(FLASH_MEM_INTERFACE_BASE + 0x54)

#define FLASH_ACR_LATENCY_SHIFT 0
#define FLASH_ACR_LATENCY_MASK 7
#define FLASH_ACR_PRFTBS (1 << 5)
#define FLASH_ACR_PRFTBE (1 << 4)
#define FLASH_ACR_PRFTEN FLASH_ACR_PRFTBE
#define FLASH_KEYR_KEY1 ((size_t)0x45670123)
#define FLASH_KEYR_KEY2 ((size_t)0xcdef89ab)

#define FLASH_OPTKEYR_KEY1 FLASH_KEYR_KEY1
#define FLASH_OPTKEYR_KEY2 FLASH_KEYR_KEY2

#define PWR_CR1 MMIO32(POWER_CONTROL_BASE + 0x00)
#define PWR_CR2 MMIO32(POWER_CONTROL_BASE + 0x04)
#define PWR_CR3 MMIO32(POWER_CONTROL_BASE + 0x08)
#define PWR_CR4 MMIO32(POWER_CONTROL_BASE + 0x0C)
#define PWR_SR1 MMIO32(POWER_CONTROL_BASE + 0x10)
#define PWR_SR2 MMIO32(POWER_CONTROL_BASE + 0x14)
#define PWR_SCR MMIO32(POWER_CONTROL_BASE + 0x18)
#define PWR_CR1_DBP (1 << 8)

void flash_prefetch_enable(void)
{
    FLASH_ACR |= FLASH_ACR_PRFTEN;
}

void flash_prefetch_disable(void)
{
    FLASH_ACR &= ~FLASH_ACR_PRFTEN;
}

void flash_set_ws(size_t ws)
{
    size_t reg32;

    reg32 = FLASH_ACR;
    reg32 &= ~(FLASH_ACR_LATENCY_MASK << FLASH_ACR_LATENCY_SHIFT);
    reg32 |= (ws << FLASH_ACR_LATENCY_SHIFT);
    FLASH_ACR = reg32;
}

void flash_unlock_option_bytes(void)
{
    FLASH_OPTKEYR = FLASH_OPTKEYR_KEY1;
    FLASH_OPTKEYR = FLASH_OPTKEYR_KEY2;
}

void pwr_disable_write(void)
{
    PWR_CR1 |= PWR_CR1_DBP;
}

void pwr_enable_write(void)
{
    PWR_CR1 &= ~PWR_CR1_DBP;
}

/* custom firmware */
struct __fmmap
{
    size_t pin0;
    size_t pin17;
    size_t rtc;
} fmemmap;

long rtc_get()
{
    return MMIO32(fmemmap.rtc);
}
