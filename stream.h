/*
 * Stream Library
*/
#pragma once
#include "lib.c"
#include "vfs.h"
#include "process.h"
#define TYPE_FILE 1
#define TYPE_KMEMORY 2 // kernel memory
#define TYPE_HANDLE 3 // OS handle
#define TYPE_UMEMORY 4 // user memory
#define TYPE_DEVICE 5 // no ungetc()

typedef struct {
    int file; // fd
    void* os_handle;
    int type; // type of stream
} stream;

typedef struct {
    struct spinlock mutex;
    struct {
        uint8_t* ptr;
        size_t size;
        size_t at;
        uint8_t flush; 
    } buffer; // allocated for writing if is locked
    void(*use)(void* data, size_t size);
} consumer;

consumer* cons_init()
{
    consumer* c = (consumer*)kalloc(sizeof(consumer), KERN_MEM);
    initlock(&(c->mutex), (char*)NULL_PTR); // un-named
    return c;
}

void cons_addbuf(consumer* cons, size_t size)
{
    cons->buffer.ptr = (uint8_t*)kalloc(size, KERN_MEM);
    cons->buffer.size = size;
}

void cons_setbuf(consumer* cons, void* buf, size_t size, int type)
{
    cons->buffer.ptr = (uint8_t*)buf;
    cons->buffer.size = size;
    cons->buffer.flush = type;
}

int cons_check_flush(consumer* cons)
{
    switch(cons->buffer.flush)
    {
    case '\n':
        return strcon((char*)cons->buffer.ptr, '\n');
    break;
    case '\0':
        return cons->buffer.at == cons->buffer.size;
    break;
    case 0xFF:
    default:
        return 0;
    break;
    }
}

void cons_flush(consumer* cons)
{
    while(cons->mutex.locked)
    {
        DNOP(); // do nothing
    }
    if(!cons_check_flush(cons))
        return;

    acquire(&(cons->mutex));
    cons->use(cons->buffer.ptr, cons->buffer.at);
    release(&(cons->mutex));
}

void cons_write(consumer* cons, void* data, size_t size)
{
    size_t sz = 0;
    if(cons->mutex.locked)
    {
        sz = min(size, cons->buffer.size - cons->buffer.at);
        memcpy(cons->buffer.ptr + cons->buffer.at, data, sz);
        cons->buffer.size += sz;
        return;
    }
    acquire(&(cons->mutex));
    cons->use(data, size);
    release(&(cons->mutex));
}

stream str_from(void* data, int data_type)
{
    stream str;
    str.type = data_type;
    switch(data_type)
    {
    case TYPE_FILE:
        str.file = prfopen(myproc(), (char*)data, F_RDWR);
    break;
    case TYPE_HANDLE:
        str.os_handle = data;
    break;
    case TYPE_KMEMORY:
        if(data > _vm(0))
            data = get_phys(data);
        str.os_handle = data;
    break;
    case TYPE_UMEMORY:
        if(data < _vm(0))
            data = vmap(data, 4096, PAGE_PRESENT | PAGE_WRITABLE, (struct vfile*)NULL_PTR);
    }
}