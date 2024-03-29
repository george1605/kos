#pragma once
#include "port.h"
#include "lib.c"
#include "smem.h"
#include "pipe.c"
#include "process.h"
#include "fs.h"
#include "modules/hashmap.h"
#define START(name, entry) struct proc* _p = prnew_k(name, 64000); _p->f = entry; prsetupvm(_p); prappend(*_p); sched();
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

#define MOD_KERNEL  0x1
#define MOD_PROC    0x2
#define MOD_DYNAMIC 0x3

struct mod_info
{
  char* name;
  void(*entry)();
  void(*exit)();
};

struct {
  struct spinlock m_lock;
  struct hashtable m_table;
} modules;

void mod_load(struct mod_info* info)
{
  acquire(&modules.m_lock);
  hashmap_set(&modules.m_table, info->name, info);
  release(&modules.m_lock);
  info->entry();
}

void mod_load_file(char* filename)
{

}

void mod_unload(char* name)
{
  struct mod_info* info = hashmap_get(&modules.m_table, name);
  info->exit();
  acquire(&modules.m_lock);
  hashmap_remove(&modules.m_table, name);
  release(&modules.m_lock);
}

struct proc* mod_as_proc(struct mod_info* info)
{
  struct proc* p = prnew_k(info->name, 256 * 1024); // 256KB
  prsetupvm(p);
  addexit(p, info->exit);
  p->f = info->exit;
  return p;
}

void mod_run(struct mod_info* info, int mode)
{
  switch(mode)
  {
  case MOD_KERNEL:
    mod_load(info);
    break;
  case MOD_PROC:
    prappend(*mod_as_proc(info));
    sched();
    break;
  }
}

typedef void(*entry_t)(void);
// loads the header and returns the entry point
// got from xv6, same source of struct buf madness
entry_t elfloadall(struct ElfHeader* hdr)
{
  struct ProgramHeader* ph = (struct ProgramHeader*)((uint8_t*)hdr + hdr->e_phoff), *endph;
  endph = ph + ph->p_memsz;
  void* pa;
  for(;ph < endph;ph++)
  {
    pa = ph->p_addr;
    readseg(pa, ph->p_filesz, ph->p_offset);
    if(ph->p_memsz > ph->p_filesz)
      stosb(pa + ph->p_filesz, 0, ph->p_memsz - ph->p_filesz);
  }
  return (entry_t)(hdr->e_entry);
}

// read ELF, load it and execute it
void elfreadexec(struct vfile* vf)
{
  struct ElfHeader* header = kalloc(sizeof(struct ElfHeader), KERN_MEM);
  entry_t entry = elfloadall(header);
  struct proc p = prcreat(vf->name);
  p.f = entry; // set the function pointer to the entry
  prappend(p); // add to the list
  sched(); // reschedule it
}

void elfload_ext(char* name, char* buffer)
{
  struct ElfHeader* header = (struct ElfHeader*)buffer;
  fs_dev->fs->read(name, buffer, fs_dev, fs_dev->priv);
  if(!checkelf(header))
    return; // if the ELF magic isn't there
    // Allocate memory for program headers
}

void elfexec_ext(char* name)
{
  char* buffer = kalloc(sizeof(struct ElfHeader), KERN_MEM); // to be changed
  elfload_ext(name, buffer);
  struct proc p = prcreat(name);
  prappend(p);
  sched();
}

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

struct ElfToken
{
  uint64_t addr;
  const char *name;
};

struct ElfHeader* pr_allocelf(struct proc* p)
{
  struct ElfHeader* head = (struct ElfHeader*)vmap(p->stack, sizeof(struct ElfHeader), 0, NULL_PTR);
  return head;
}

void reallocelf(struct ElfHeader* header)
{
  header->e_entry = vmap(header->e_entry, sizeof(void*), 0, NULL_PTR);
}

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

struct SoHeader *readso(int *ptr) // int ptr[6]
{
  if (!ptr)
    return 0;

  struct SoHeader *i = TALLOC(struct SoHeader);
  i->e_ident[0] = 0x7f;
  i->e_ident[1] = 'S';
  i->e_ident[2] = 'O';
  i->e_ident[3] = ptr[0];
  i->e_ident[4] = ptr[1];

  i->e_sect[0] = vfsmap(".data", _vm(ptr[3]));
  i->e_sect[1] = vfsmap(".rodata", _vm(ptr[4]));
  i->e_sect[2] = vfsmap(".text", _vm(ptr[5]));
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

uint8_t elf_start(uint8_t *buf, elf_priv_data *priv)
{
	ElfHeader *header = (ElfHeader*)buf;
	mprint("Type: %s%s%s\n",
		header->e_ident[4] == 1 ? "32bit ":"64 bit",
		header->e_ident[5] == 1 ? "Little Endian ":"Big endian ",
		header->e_ident[6] == 1 ? "True ELF ":"buggy ELF ");
	if(header->e_type != 2)
	{
		kprintf("File is not executable!\n");
		return 0;
	}
	/* Map the virtual space */
	uint32_t phys_loc = heapbrk;
	uint32_t cr3 = kalloc(4096, KERN_MEM);
	/* Find first program header and loop through them */
	ProgramHeader *ph = (ProgramHeader *)(buf + header->e_phoff);
	for(int i = 0; i < header->e_phnum; i++, ph++)
	{
		switch(ph->p_type)
		 {
		 	case 0: /* NULL */
		 		break;
		 	case 1: /* LOAD */
		 		printf("LOAD: offset 0x%x vaddr 0x%x paddr 0x%x filesz 0x%x memsz 0x%x\n",
		 				ph->p_offset, ph->p_vaddr, ph->p_paddr, ph->p_filesz, ph->p_memsz);
		 		map_page(ph->p_vaddr, phys_loc, PAGE_PRESENT | PAGE_WRITABLE);
		 		memcpy(ph->p_vaddr, buf + ph->p_offset, ph->p_filesz);
		 		break;
      case :

        break;
		 	default: 
		 	 kprint("Unsupported p_type! Bail out!\n");
		 	 return 0;
		 }
	}
	/* Program loaded, jump to execution */
	START("elf", header->e_entry);
}
