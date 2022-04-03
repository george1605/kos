#pragma once
#include "lib.c"
#include "smem.h"
#include "pipe.c"
#include "vfs.h"
#define ELF_32 2
#define ELF_64 4

#define ELF_LITTLE 8
#define ELF_BIG 16

#define ELF_TRUE 32
#define ELF_BUGGY 64
typedef uint32_t Elf32_Addr;
typedef uint32_t Elf32_Off;
typedef uint16_t Elf32_Half;
typedef uint32_t Elf32_Word;
typedef uint8_t Elf32_Char;
typedef uint64_t Elf64_Addr;
typedef uint64_t Elf64_Xword;
typedef uint64_t Elf64_Off;
typedef uint16_t Elf64_Half;
typedef uint32_t Elf64_Word;
typedef uint8_t Elf64_Char;
typedef int Elf32_Sword;

struct ElfHeader
{
  unsigned char e_ident[16];
  Elf32_Half e_type;
  Elf32_Half e_machine;
  Elf32_Word e_version;
  Elf32_Addr e_entry;
  Elf32_Off e_phoff;
  Elf32_Off e_shoff;
  Elf32_Word e_flags;
  Elf32_Half e_ehsize;
  Elf32_Half e_phentsize;
  Elf32_Half e_phnum;
  Elf32_Half e_shentsize;
  Elf32_Half e_shnum;
  Elf32_Half e_shstrndx;
};

struct ProgramHeader
{
  size_t p_type;
  size_t p_flags;
  long p_offset;
  long p_addr;
  long p_filesz;
  long p_memsz;
};

struct ElfSection
{
  struct ElfHeader *header;
  struct ProgramHeader *pheader;
  char *name;
  int active;
};

struct ElfMemory
{
  void *data;
  void *bss;
  void *text;
  size_t size;
};

struct ElfSymbol
{
  Elf32_Word st_name;
  Elf32_Addr st_value;
  Elf32_Word st_size;
  unsigned char st_info;
  unsigned char st_other;
  struct ElfSection st_shndx;
};

void readsym(struct ElfSymbol u, char bytes[])
{
}

typedef struct ElfSymbol *ElfSTable;

struct ElfRel
{
  Elf32_Addr r_offset;
  Elf32_Word r_info;
};

struct ElfDynamic
{
  Elf32_Sword d_tag;
  union
  {
    Elf32_Word d_val;
    Elf32_Addr d_ptr;
  } d_un;
};

int checkemem(struct ElfMemory u)
{
  if (u.data < u.bss && u.text < u.data)
    return 1;
  return 0;
}

struct ElfMemory elfalloc(size_t bytes)
{
  struct ElfMemory u;
  void *k = alloc(0, bytes);
  u.size = bytes;
  u.text = k;
  u.data = k + bytes / 3;
  u.bss = k + 2 * (bytes / 3);
  return u;
}

void setupelf(struct ElfHeader *hdr)
{
  char u[10] = {0x7f, 'E', 'L', 'F', 1, 2, 1, 1, 1};
  memcpy(hdr->e_ident, hdr, 8);
}

struct ElfHeader *readelf(int *u)
{
  struct ElfHeader *hdr = 0;
  if (u != 0)
  {
    hdr = (struct ElfHeader *)kalloc(sizeof(struct ElfHeader), KERN_MEM);
    setupelf(hdr);
    hdr->e_ident[0x7] = 0x14;
    hdr->e_version = 10;
    hdr->e_flags = *(u + 1);
    hdr->e_entry = *u;
    hdr->e_phoff = *(u + 2);
    hdr->e_shoff = *(u + 3);
  }
  return hdr;
}

void freelf(struct ElfSection *u)
{
  if (u != 0)
  {
    free((int *)u->header);
    free((int *)u->name);
    u->header = (struct ElfHeader *)0;
  }
}

struct proc execelf(struct ElfSection *u)
{
  struct proc pr;
  if (u->header->e_version > 8)
  {
    prinit(pr, 0); // creates a
    u->active = 1;
  }
  else
  {
    pr.pid = 0;
  }
  return pr;
}

void killelf(struct proc _process, struct ElfSection *_elf)
{
  if (_process.pid != 0 && _elf != 0)
  { // SECURITY MEASURE
    prkill(_process);
    _elf->active = 0;
  }
}

void *checkelf(uint8_t *buffer)
{
  struct ElfHeader *header = (struct ElfHeader *)buffer;
  if (header->e_ident[0] == 0x7f &&
      header->e_ident[1] == 'E' && header->e_ident[2] == 'L' && header->e_ident[3] == 'F')
  {
    return (void *)1;
  }
  return (void *)0;
}

long getelf(struct ElfHeader *header)
{
  long u = (long)1;
  u |= (header->e_ident[4] == 1 ? ELF_32 : ELF_64);
  u |= (header->e_ident[5] == 1 ? ELF_LITTLE : ELF_BIG);
  u |= (header->e_ident[6] == 1 ? ELF_TRUE : ELF_BUGGY);
  return u;
}

struct SoHeader
{
  int e_ident[12];
  struct vfile e_sect[3];
  void (*somain)(int);
};

// Info | .Data | .RoData | .Text

struct SoHeader *readso(int *ptr)
{
  if (!ptr)
    return 0;

  struct SoHeader *i = TALLOC(struct SoHeader);
  i->e_ident[0] = 0x7f;
  i->e_ident[1] = 'S';
  i->e_ident[2] = 'O';
  i->e_ident[3] = ptr[0];
  i->e_ident[4] = ptr[1];

  i->e_sect[0] = vfsmap(".data", ptr[3] + VM_NUM);
  i->e_sect[1] = vfsmap(".rodata", ptr[4] + VM_NUM);
  i->e_sect[2] = vfsmap(".text", ptr[5] + VM_NUM);
  return i;
}

void attachso(struct SoHeader *so, struct proc p) // pushes the sections onto the process stack
{
  int *u = (int *)p.stack;
  u[0] = (int)so->e_sect[0].mem;
  u[1] = (int)so->e_sect[1].mem;
  u[2] = (int)so->e_sect[2].mem;
}

struct ElfSymbol getsym(int *ptr)
{
  struct ElfSymbol i;
  i.st_value = ptr[0] - 0xFF;
  i.st_size = ptr[1];
  i.st_name = (size_t)ptr[2];
  return i;
}