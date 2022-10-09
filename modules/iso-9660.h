#include "../fs.h"
#define ISO_RES_START 0x0
#define ISO_RES_END 0x7

struct ISOHeader
{
  char typecode;
  char ident[5]; // 'CD001'
  char version;
  char unused; // 0x0
  char s_ident[32];
  char v_ident[32];
  char unused2[8];
  
} __attribute__((packed));

struct ISOVoldesc
{
  uint8_t type;
  uint8_t magic[5];
  uint8_t version;
} __attribute__((packed));

struct ISODate2
{
  uint8_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  uint8_t offset;
} __attribute__((packed));


void iso_init(struct ISOHeader* hdr)
{
  if(hdr == NULL_PTR) return; 
  hdr->typecode = 0x1;
  memcpy(hdr->ident, "CD001", 5);
  hdr->version = 0x1;
  hdr->unused = 0x0;
  hdr->unused2[0] = 0x0;
}

void iso_write(struct ISOHeader* hdr)
{
  
}
