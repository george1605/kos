#pragma once 
#include "../port.h"
#include "../lib.c"
#include "../mem.h"
#ifndef _ASSERT
  #define _ASSERT(x) if(!x) return 0;
#endif
#define ATAPI_SECTOR_SIZE 2048 //2K
 
#define ATA_IRQ_PRIMARY     0x0E
#define ATA_IRQ_SECONDARY   0x0F
 
#define ATA_DATA(x)         (x)
#define ATA_FEATURES(x)     (x+1)
#define ATA_SECTOR_COUNT(x) (x+2)
#define ATA_ADDRESS1(x)     (x+3)
#define ATA_ADDRESS2(x)     (x+4)
#define ATA_ADDRESS3(x)     (x+5)
#define ATA_DRIVE_SELECT(x) (x+6)
#define ATA_COMMAND(x)      (x+7)
#define ATA_DCR(x)          (x+0x206)   
 
#define ATA_BUS_PRIMARY     0x1F0
#define ATA_BUS_SECONDARY   0x170
#define ATA_DRIVE_MASTER    0xA0
#define ATA_DRIVE_SLAVE     0xB0

#define ATA_SELECT_DELAY(bus) \
  { inb(ATA_DCR(bus)); inb(ATA_DCR(bus)); inb(ATA_DCR(bus)); inb(ATA_DCR(bus)); } 

struct ata_task {
  struct ata_task* next;
  size_t status;
  size_t tid;
  size_t bus;
  size_t drive;
  size_t lba;
  void(*callio)(struct ata_task* _This);
} *cataproc;

void wait_atask(struct ata_task* u){
  while(u->status != 0);
}

void ata_next(void){
  if(cataproc->status == 0){
    if(cataproc->next != 0){
      cataproc = cataproc->next;
      HALT();
      cataproc->callio(cataproc);
    }
  }
}

void ata_end(){
  while(cataproc->next != 0)
    ata_next();
}

void ata_grab(void){
   
}
 
void ata_sreset(size_t bus){
  outb (0x02, ATA_DCR (bus));
  outb (0x00, ATA_DCR (bus));
}

void ata_release(){
  cataproc->status = 0;
  ata_next();
}

int atapi_reads (size_t bus, size_t drive, size_t lba, uint8_t *buffer){
	uint8_t read_cmd[12] = { 0xA8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	uint8_t status;
	int size;
	ata_grab();
	outb (drive & (1 << 4), ATA_DRIVE_SELECT (bus));      
	ATA_SELECT_DELAY(bus);       
	outb (0x0, ATA_FEATURES (bus));     
	outb (ATAPI_SECTOR_SIZE & 0xFF, ATA_ADDRESS2 (bus));
	outb (ATAPI_SECTOR_SIZE >> 8, ATA_ADDRESS3 (bus));
	outb (0xA0, ATA_COMMAND (bus));   
	while ((status = inb (ATA_COMMAND (bus))) & 0x80) 
	PAUSE();
	while (!((status = inb (ATA_COMMAND (bus))) & 0x8) && !(status & 0x1))
	PAUSE();

	if (status & 0x1) {
	  size = -1;
	  goto cleanup;
	}
	read_cmd[9] = 1;              /* 1 sector */
	read_cmd[2] = (lba >> 0x18) & 0xFF;   /* most sig. byte of LBA */
	read_cmd[3] = (lba >> 0x10) & 0xFF;
	read_cmd[4] = (lba >> 0x08) & 0xFF;
	read_cmd[5] = (lba >> 0x00) & 0xFF;   /* least sig. byte of LBA */
	/* Send ATAPI/SCSI command */
	outsw (ATA_DATA (bus), (uint16_t *) read_cmd, 6);
	size =
	(((int) inb (ATA_ADDRESS3 (bus))) << 8) |
	(int) (inb (ATA_ADDRESS2 (bus)));
	
	_ASSERT(size == ATAPI_SECTOR_SIZE);
	insw (ATA_DATA (bus), buffer, size / 2);
	while ((status = inb (ATA_COMMAND (bus))) & 0x88) 
	PAUSE();
	cleanup:
	  ata_release();
    
	return size;
}

void ata_init(){
  cataproc = TALLOC(struct ata_task); 
  cataproc->tid = 1;
}

void ata_rdio(struct ata_task* _This){
  uint8_t* atabuf = (uint8_t*)0x9F0000;
  int res = atapi_reads(_This->bus, _This->drive, _This->lba, atabuf);
  _This->status = 0;
}
