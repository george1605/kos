#pragma once
#include "lib.c"
#include "port.h"
int *SMI_CMD;
char ACPI_ENABLE;
char ACPI_DISABLE;
int *PM1a_CNT;
int *PM1b_CNT;
short SLP_TYPa;
short SLP_TYPb;
short SLP_EN;
short SCI_EN;
char PM1_CNT_LEN;

struct rsdp_desc
{
  char sign[8];
  uint8_t checksum;
  char OEMID[6];
  uint8_t revision;
  uint32_t raddr;
} __attribute__((packed));

const char *rsdp_sign = "RSD PTR ";

struct rsdp_desc* rsdp_find()
{
  int a;
  for (a = 0x01; a < 0x7BFF; a += 16)
    if (strcmp(rsdp_sign, (char*)a) == 0)
      return (struct rsdp_desc *)a;

  for (a = 0xE0000; a < 0xFFFFF; a += 16)
    if(strcmp(rsdp_sign,a) == 0)
      return (struct rsdp_desc *)a;

  return (struct rsdp_desc*)NULL_PTR;
}

int acpi_enable(void)
{
  if ((inw((unsigned int)PM1a_CNT) & SCI_EN) == 0)
  {
    // check if acpi can be enabled
    if (SMI_CMD != 0 && ACPI_ENABLE != 0)
    {
      outb((unsigned int)SMI_CMD, ACPI_ENABLE); 
      int i;
      for (i = 0; i < 300; i++)
      {
        if ((inw((unsigned int)PM1a_CNT) & SCI_EN) == 1)
          break;
        delay(10);
      }
      if ((int)PM1b_CNT != 0)
        for (; i < 300; i++)
        {
          if ((inw((unsigned int)PM1b_CNT) & SCI_EN) == 1)
            break;
          delay(10);
        }
      if (i < 300)
      {
        return 0;
      }
      else
      {
        return -1;
      }
    }
    else
    {
      return -1;
    }
  }
  else
  {
    return 0;
  }
}

void acpi_shutdown(void)
{
  if (SCI_EN == 0)
    return;

  acpi_enable();
  outw((unsigned int)PM1a_CNT, SLP_TYPa | SLP_EN);
  if (PM1b_CNT != 0)
    outw((unsigned int)PM1b_CNT, SLP_TYPb | SLP_EN);
}

void acpi_reset()
{
    uint8_t good = 0x02;
    while (good & 0x02)
        good = inportb(0x64);
    outportb(0x64, 0xFE);
}

void acpi_init(void)
{
  acpi_enable();
}