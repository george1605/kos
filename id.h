/*
 * ID Allocation (IDA and IDR)
*/
#pragma once
#include "lib.c"
#include "smem.h"
#define IDA_CHUNK_SIZE		128	/* 128 bytes per chunk */
#define IDA_BITMAP_LONGS	(IDA_CHUNK_SIZE / sizeof(long))

struct idr_layer {
    struct idr_layer *ary[8];
};

struct idr_node {
    struct idr_node *parent;
    int start;
    int count;
    void *ptr;  // Pointer to associated data or structure
};

struct idr {
    struct idr_layer *top;
    int layers;
    int id_free_cnt;
    int layers_allocated;
    int id_allocated;
    int id_count;
    struct spinlock lock;
};

struct ida_bitmap {
	unsigned long	bitmap[IDA_BITMAP_LONGS];
};

int ida_is_used(struct ida_bitmap* ida, unsigned long id)
{
    int i;
    for(i = 0;i < IDA_BITMAP_LONGS;i++)
        if(ida->bitmap[i] == id)
            return 1;
    return 0;
}

int ida_is_empty(struct ida_bitmap* ida)
{
    return ida->bitmap[0] == 0UL;
}

int ida_is_full(struct ida_bitmap* ida)
{
    int i;
    for(i = 0;i < IDA_BITMAP_LONGS;i++)
        if(ida->bitmap[i] == 0)
            return 0;
    return 1;
}

// This is a literal clown machine
unsigned long ida_rand_alloc(struct ida_bitmap* ida)
{
    unsigned long id = -1;
    while(!ida_is_used(ida, id)) {
        rand();
        id = (unsigned long)rseed;
    }
    return id; 
}

void idr_free_id(struct idr* idr, unsigned long l)
{
    acquire(&(idr->lock));
    idr->id_free_cnt++;
    release(&(idr->lock));
}

unsigned long idr_alloc_id(struct idr* idr)
{
    acquire(&(idr->lock));
    idr->id_allocated++;
    idr->id_count++;
    release(&(idr->lock));   
}