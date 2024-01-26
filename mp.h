#pragma once
#include "port.h"
#include "ioapic.h"
#include "cpuem.h"
#define MPPROC    0x00
#define MPBUS     0x01
#define MPIOAPIC  0x02
#define MPIOINTR  0x03
#define MPLINTR   0x04
#define MPBOOT    0x02

struct mp
{                     
  uint8_t sign[4]; 
  void *physaddr;     
  uint8_t length;       // 1
  uint8_t specrev;      // [14]
  uint8_t checksum;     // all bytes must add up to 0
  uint8_t type;         // MP system config type
  uint8_t imcrp;
  uint8_t reserved[3];
};

struct mpioapic 
{
  uint8_t type; 
  uint8_t id;
  uint8_t version;
  uint8_t flags; 
  uint32_t address;
};

struct mpconf {         // configuration table header
  uint8_t signature[4];           // "PCMP"
  uint16_t length;                // total table length
  uint8_t version;                // [14]
  uint8_t checksum;               // all bytes must add up to 0
  uint8_t product[20];            // product id
  uint32_t *oemtable;               // OEM table pointer
  uint16_t oemlength;             // OEM table length
  uint16_t entry;                 // entry count
  uint32_t *lapicaddr;              // address of local APIC
  uint16_t xlength;               // extended table length
  uint8_t xchecksum;              // extended table checksum
  uint8_t reserved;
};

struct mpproc {         // processor table entry
  uint8_t type;                   // entry type (0)
  uint8_t apicid;                 // local APIC id
  uint8_t version;                // local APIC verison
  uint8_t flags;                  // CPU flags
  uint8_t signature[4];           // CPU signature
  uint32_t feature;                 // feature flags from CPUID instruction
  uint8_t reserved[8];
};

struct mp* mpsearchx(size_t a, int len)
{
  uint8_t *e, *p, *addr;

  addr = (uint8_t*)_vm(a);
  e = addr + len;
  for (p = addr; p < e; p += sizeof(struct mp))
    if (memncmp(p, "_MP_", 4) == 0 && sum(p, sizeof(struct mp)) == 0)
      return (struct mp*)p;
  return 0;
}

struct mp* mpsearch(){
  uint8_t *bda;
  uint32_t p;
  struct mp *mp;

  bda = (uint8_t *)_vm(0x400);
  if ((p = ((bda[0x0F] << 8) | bda[0x0E]) << 4)) // address 0x800040F
  {
    if ((mp = mpsearchx(p, 1024)))
      return mp;
  }
  else
  {
    p = ((bda[0x14] << 8) | bda[0x13]) * 1024;
    if ((mp = mpsearchx(p - 1024, 1024)))
      return mp;
  }
  return mpsearchx(0xF0000, 0x10000);
}

struct mpconf* mpconfig(struct mp **pmp)
{
  struct mpconf *conf;
  struct mp *mp;

  if((mp = mpsearch()) == 0 || mp->physaddr == 0)
    return 0;
  conf = (struct mpconf*)_vm((uint32_t)mp->physaddr);
  if(memncmp(conf, "PCMP", 4) != 0)
    return 0;
  if(conf->version != 1 && conf->version != 4)
    return 0;
  if(sum((uint8_t*)conf, conf->length) != 0)
    return 0;
  *pmp = mp;
  return conf;
}

extern void sysc_handler(struct regs* r);

void load_processor_info(void) {
	unsigned long a, b, unused;
	cpuid(0,unused,b,unused,unused);

	struct cpu* cpu = mycpu();

	if (b == 0x756e6547) {
		cpuid(1, a, b, unused, unused);
    printf("Intel CPU %i, Model %i, Family %i.\n", cpu->apicid, (a >> 4) & 0x0F, (a >> 8) & 0x0F);
	} else if (b == 0x68747541) {
		cpuid(1, a, unused, unused, unused);
	}
	/* See if we can get a long manufacturer strings */
	cpuid(0x80000000, a, unused, unused, unused);
  uint32_t brand[12];
	if (a >= 0x80000004) {
		cpuid(0x80000002, brand[0], brand[1], brand[2], brand[3]);
		cpuid(0x80000003, brand[4], brand[5], brand[6], brand[7]);
		cpuid(0x80000004, brand[8], brand[9], brand[10], brand[11]);
	}

	uint32_t efer_hi, efer_lo;
	asm volatile ("rdmsr" : "=d"(efer_hi), "=a"(efer_lo) : "c"(0xc0000080));    /* Read current EFER */
	asm volatile ("wrmsr" : : "c"(0xc0000080), "d"(efer_hi), "a"(efer_lo | 1)); /* Enable SYSCALL/SYSRET in EFER, after sw to long mode */
	asm volatile ("wrmsr" : : "c"(0xC0000081), "d"(0x1b0008), "a"(0));          /* Set segment bases in STAR */
	asm volatile ("wrmsr" : : "c"(0xC0000082),                                  /* Set SYSCALL entry point in LSTAR */
	              "d"((uint64_t)&sysc_handler >> 32),
	              "a"((uint64_t)&sysc_handler & 0xFFFFffff));
	asm volatile ("wrmsr" : : "c"(0xC0000084), "d"(0), "a"(0x700));             /* SFMASK: Direction flag, interrupt flag, trap flag are all cleared */
}

int ncpu;
void mpinit(){
  uint8_t *p, *e;
  int ismp;
  struct mp *mp;
  struct mpconf *conf;
  struct mpproc *proc;
  struct mpioapic *ioapic;

  if((conf = mpconfig(&mp)) == 0)
    perror("Expect to run on an SMP");
  ismp = 1;
  lapic = (size_t*)conf->lapicaddr;
  for(p=(uint8_t*)(conf+1), e=(uint8_t*)conf+conf->length; p<e; ){
    switch(*p){
    case MPPROC:
      proc = (struct mpproc*)p;
      if(ncpu < NCPU) {
        cpus[ncpu].apicid = proc->apicid;  // apicid may differ from ncpu
        ncpu++;
      }
      p += sizeof(struct mpproc);
      continue;
    case MPIOAPIC:
      ioapic = (struct mpioapic*)p;
      ioapicid = ioapic->id;
      p += sizeof(struct mpioapic);
      continue;
    case MPBUS:
    case MPIOINTR:
    case MPLINTR:
      p += 8;
      continue;
    default:
      ismp = 0;
      break;
    }
  }
  if(!ismp)
    perror("Could not find a suitable machine.");

  if(mp->imcrp){
    outb(0x22, 0x70);   // Select IMCR
    outb(0x23, inb(0x23) | 1);
  }
}