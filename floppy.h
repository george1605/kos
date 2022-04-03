#pragma once
#include "irq.c"
#include "port.h"

#define FLOPPY_DOR  2  // digital output register
#define FLOPPY_MSR  4  // master status register, read only
#define FLOPPY_FIFO 5  // data FIFO, in DMA operation for commands
#define FLOPPY_CCR  7   
#define FLOPPY_DRNO -1
#define FLOPPY_SDRNO -2

int* floppyirq = (int*)0xDDFF00;
char *floppytype[6] = { "no floppy drive", 
                       "360kb 5.25in floppy drive",
                       "1.2mb 5.25in floppy drive",
                       "720kb 3.5in",
                       "1.44mb 3.5in", 
                       "2.88mb 3.5in"};

void ndelay(int u){
  int i = 0;
  while(i < u){
    i++;
    i = i * 2;
    i = i / 2;
  }
}

uint8_t detfloppy()
{
  uint8_t c;
  outportb(0x70, 0x10);
  c = inportb(0x71);
  return c;
}

uint8_t getfloppy(int flno)
{
  int u = detfloppy();
  if(flno == -1)
   return (uint8_t)(u >> 4); 
  else
   return (uint8_t)(u & 0xF); 
}

enum floppyreg
{
   STATUS_REGISTER_A                = 0x3F0, // read-only
   STATUS_REGISTER_B                = 0x3F1, // read-only
   DIGITAL_OUTPUT_REGISTER          = 0x3F2,
   TAPE_DRIVE_REGISTER              = 0x3F3,
   MAIN_STATUS_REGISTER             = 0x3F4, // read-only
   DATARATE_SELECT_REGISTER         = 0x3F4, // write-only
   DATA_FIFO                        = 0x3F5,
   DIGITAL_INPUT_REGISTER           = 0x3F7, // read-only
   CONFIGURATION_CONTROL_REGISTER   = 0x3F7  // write-only
};

void wrfloppy(int base, char cmd)
{
  int i; 
  for (i = 0; i < 600; i++)
  {
    ndelay(10000); 
    if (0x80 & inportb(base + FLOPPY_MSR))
    {
      outportb(base + FLOPPY_FIFO, cmd);
    }
  }
}

uint8_t rdfloppy(int base)
{

  int i; 
  for (i = 0; i < 600; i++)
  {
    ndelay(10000); 
    if (0x80 & inportb(base + FLOPPY_MSR))
    {
      return inportb(base + FLOPPY_FIFO);
    }
  }
  return 0; // not reached
}

void hndfloppy(struct regs* r){
  *floppyirq = 1;
}

void initfloppy(){
  irq_install_handler(6,hndfloppy);
}