#pragma once
#include "fs.h"
#include "process.h"
/*
  --------------------------- (void*)
  Function name | Args | Body
*/
#define S_UNKNOWN 0x0
#define S_STDLIB 0x10
#define S_DRIVER 0x20
#define S_PLDEP 0x40
#define S_EMMBED 0x80
#define S_KERNEL 0xF0

#define S_FUNC 0x1
#define S_VAR 0x2
#define S_MACRO 0x3
typedef void (*shentry)(void);

typedef struct _shl
{
	int flags;
	int version;
	int faddr;
	char info[12];
} shl;

typedef struct _senv
{
	struct proc process;
} shlenv;

typedef struct _ssym
{
	char *name;
	void *value;
	int type;
} shlsym;

void shlset(shl *u)
{
	u->flags = S_STDLIB;
	u->info[0] = 0x7F;
	u->info[1] = 'E';
	u->info[2] = 'L';
	u->info[3] = 'F';
	u->info[4] = 32; // 32-bit
}

struct buf *shlget(shl x)
{
	int a;
	struct buf *u = TALLOC(struct buf);
	u.flags |= B_DIRTY;
	memcpy(u->data, x.info, 12);
	x->data[13] = x.flags;
	return u;
}

void shladd(shlsym u, struct buf *b, int offset)
{
	memcpy(b->data + offset, u.name, strlen(u.name));
}

shlsym shlload(struct buf *b, int offset)
{
	shlsym i;
	if(b->data[0] == 0)
		bread(b, 0);

	i.name = (char*)(b->data + 4);
	i.type = bcasti(b, 16); // 12 bytes
	i.value = (void*)(b->data + 20);
	return i;
}