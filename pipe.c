#pragma once
#include "process.h"
#include "vfs.h"
#define NPIPES 16

void pipesend(int from,int to){ 
  struct buf* b = TALLOC(struct buf);
  _read(from,b,512);
  _write(to,b,512);
  brelse(b);
}

struct mailbox {
  struct proc* mprocs;
  void* memory;
  int msize;
};

struct procmsg {
  int type;
  char* msg; // optional
  struct proc from;
  struct proc to;
};

struct shbuf {
  char* data;
  size_t size;
  size_t point;
  struct spinlock lock;
};

void shbputc(struct shbuf x,char u){
  if(x.lock.locked || x.point == x.size - 1) return; // already used by a file

  acquire(&x.lock);
  x.data[++x.point] = u;
  release(&x.lock);
}

void shbgetio(struct shbuf x,int port){
  shbputc(x,inb(port));
}

void shbread(struct shbuf x,int fd){
  struct buf* k = kalloc(sizeof(struct buf),KERN_MEM);
  int minsz = min(x.size, 512);
  memcpy(x.data, k->data, minsz);
  free(k);
}

struct kpipe
{
  struct circbuf buf;
  struct proc* procs[2];
  int fds[2];
} pipes[NPIPES];

int pipesetup()
{
  for(int i = 0;i < NPIPES;i++) {
    if(pipes[i].fds[0] == 0) {
      pipes[i].fds[0] = fdalloc();
      pipes[i].fds[1] = fdalloc();
      return i;
    }
  }
  return -1;
}

void sethead(struct proc* p, struct circbuf cbuf);
void settail(struct proc* p, struct circbuf cbuf);

void pipeattach(int pipeno, struct proc* p1, struct proc* p2)
{
  pipes[pipeno].procs[0] = p1;
  pipes[pipeno].procs[1] = p2;
  sethead(p1, pipes[pipeno].buf); // sets the head and tail
  settail(p2, pipes[pipeno].buf);
}

void pipewrite(int pipeno, char* buf, size_t size)
{
  if(pipes[pipeno].procs[1]->pid == 0)
    pipes[pipeno].procs[1] = myproc();
  for(int i = 0;i < size;i++)
    pushcbuf(&pipes[pipeno].buf, buf[i]);
}

void pipewrite(int pipeno, char* buf, size_t size)
{
  if(pipes[pipeno].procs[0]->pid == 0)
    pipes[pipeno].procs[0] = myproc();
  for(int i = 0;i < size;i++)
    buf[i] = popcbuf(&pipes[pipeno].buf, i);
}

void prhandle(struct procmsg u){ // the default process message handler 
  switch(u.type){
    case 0x20:
      prkill(&u.to);
    break;
    case 0xF:
      prcpy(u.from, u.to);
    break;
    case 0xA:
      prdup(u.to);
    break;
    case 0x11:
    
    break;
    default:
      return;
    break;
  }
}

void prsend(struct proc to, struct proc from, int type){
  struct procmsg msg;
  msg.from = from;
  msg.to = to;
  msg.type = type;
  msg.msg = (char*)0;
  prhandle(msg);
}

struct mailbox mshare(void* mem,int bytes){
  struct mailbox u;
  if(mem != 0 && *(int*)(mem+bytes) != 0xdeadbeef){
   u.memory = mem;
   u.msize = bytes;
  }
  return u;
} 

void mcopy(struct mailbox m,char* bytes,int offset){
  int i = 0;
  if(offset > m.msize)
    offset = 0;
  while(bytes[i] != 0 && bytes[i] != 0xdeadbeef){
      *(int*)(m.memory + offset) = bytes[i];
      i++;
  }
}