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
      memset(pipes[i].procs, 0, 2 * sizeof(struct proc*));
      pipes[i].fds[0] = fdalloc();
      pipes[i].fds[1] = fdalloc();
      return i;
    }
  }
  return -1;
}

void sethead(struct proc* p, int fd, struct circbuf cbuf)
{
  p->ofiles[fd].flags = F_PIPE & F_READ;
}

void settail(struct proc* p, int fd, struct circbuf cbuf)
{
  p->ofiles[fd].flags = F_PIPE & F_WRITE;
}

void pipeattach(int pipeno, struct proc* p1, struct proc* p2)
{
  pipes[pipeno].procs[0] = p1;
  pipes[pipeno].procs[1] = p2;
  sethead(p1, pipes[pipeno].fds[0], pipes[pipeno].buf); // sets the head and tail
  settail(p2, pipes[pipeno].fds[1], pipes[pipeno].buf);
}

void pipewrite(int pipeno, char* buf, size_t size)
{
  if(pipes[pipeno].procs[1] == NULL_PTR)
    pipes[pipeno].procs[1] = myproc();
  for(int i = 0;i < size;i++)
    pushcbuf(&pipes[pipeno].buf, buf[i]);
  signal(SIG_WRITEPIPE, pipes[pipeno].procs[0], pipes[pipeno].fds[1], buf);
}

void piperead(int pipeno, char* buf, size_t size)
{
  if(pipes[pipeno].procs[0] == NULL_PTR)
    pipes[pipeno].procs[0] = myproc();
  for(int i = 0;i < size;i++)
    buf[i] = readcbuf(&pipes[pipeno].buf, i);
}

void pipeclose(int pipeno)
{
  struct proc* p1 = pipes[pipeno].procs[0], *p2 = pipes[pipeno].procs[1];
  int fd1 = pipes[pipeno].fds[0], fd2 = pipes[pipeno].fds[1];

  memset(&(p1->ofiles[fd1]), 0, sizeof(struct file)); // clear the entries  
  memset(&(p2->ofiles[fd2]), 0, sizeof(struct file));  

}

void pipestd(struct proc* p1, struct proc* p2)
{
  vfslink2(&(p1->std[0]), &(p2->std[1]));
  vfslink2(&(p1->std[1]), &(p2->std[0]));
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

typedef void(*rd_func)(void*);
typedef void(*wr_func)(void*);

struct rdpipe
{
  wr_func   wr;
  rd_func*  rd;
  size_t num_rd, max_rd;
  void* handle;
  struct spinlock lock;
};

void rdpipe_switch(struct rdpipe* pipe, int id)
{
  if(id > pipe->num_rd) return;
  acquire(&pipe->lock);
  wr_func temp = pipe->wr;
  pipe->wr = pipe->rd[id];
  pipe->rd[id] = temp;
  release(&pipe->lock);
}

void rdpipe_add(struct rdpipe* pipe, rd_func reader)
{
  if(pipe->num_rd == pipe->max_rd) return;
  pipe->rd[pipe->num_rd++] = reader;
}

void rdpipe_setup(struct rdpipe* pipe, wr_func writer)
{
  pipe->wr = writer;
}

// set the writer as an interrup function basically
void rdpipe_int(struct rdpipe* pipe, int irq_no)
{
  irq_install_handler(pipe->wr, irq_no);
}

// signal to the readers that new data has arrived
void rdpipe_signal(struct rdpipe* pipe)
{
  for(int i = 0;i < pipe->num_rd;i++)
    pipe->rd[i](pipe->handle);
}

#define RDPIPE_SET    0x1
#define RDPIPE_STRCPY 0x2

void rdpipe_write(struct rdpipe* pipe, void* data, int type)
{
  switch(type)
  {
  case RDPIPE_SET:
    pipe->handle = data;
    break;
  case RDPIPE_STRCPY:
    strcpy(pipe->handle, data);
    break;
  } 
  rdpipe_signal(pipe);
}