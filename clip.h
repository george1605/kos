#pragma once 
#include "mem.h"
#include "fs.h"
char* clipboard;

void copyclip(const char* u, size_t bytes){
  int i;
  if(clipboard != 0){
    memcpy(clipboard,u,bytes);
  }
}

void freeclip(){
  memset(clipboard,0,128);
}

void initclip(){
  clipboard = kalloc(128, KERN_MEM);
}

void saveclip(int fd){ // copies the data from clipboard and writes to disk
  struct buf* bp = kalloc(sizeof(struct buf), KERN_MEM);
  memcpy(bp->data,clipboard,128);
  _write(fd,bp,128);
  brelse(bp);
}