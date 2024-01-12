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
    struct idr_layer *ary[8]; // NULL
    size_t start;
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
	unsigned long bitmap[IDA_BITMAP_LONGS];
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

int idr_is_leaf(struct idr_layer* layer)
{
    for(int i = 0;i < 8;i++)
        if(layer->ary[i] != NULL_PTR)
            return 0;
    return 1;
}

void idr_traverse(struct idr_layer* layer, struct idr_node** leafs)
{
    int i, j = 0;
    for(i = 0;i < 8;i++)
    {
        if(!idr_is_leaf(layer->ary[i]))
            idr_traverse(layer->ary[i], leafs);
        else {
            leafs = &leafs[j++];
            leafs[0] = (struct idr_node*)layer->ary[i];
        }
    }
}

int idr_add_id(struct idr_layer* layer, unsigned long id)
{
    int i;
    for(i = 0;i < 8;i++)
    {
        if(layer->ary[i] == NULL_PTR)
        {
            layer->ary[i] = kalloc(sizeof(struct idr_node), KERN_MEM);
            struct idr_node* node = (struct idr_node*)layer->ary[i];
            node->start = id;
            return 1;
        }
    }
    for(i = 0;i < 8;i++)
        if(idr_add_id(layer->ary[i], id))
            return 1;
    return 0;
}

int idr_is_full(struct idr* idr)
{
    return idr->id_count == idr->id_allocated && idr->id_free_cnt == 0;
}

struct idr_node* idr_find(struct idr* idr, unsigned long id)
{
    struct idr_node** nodes = (struct idr_node**)kalloc(sizeof(struct idr_node) * 20, KERN_MEM);
    struct idr_node* found = (struct idr_node*)NULL_PTR;
    idr_traverse(idr->top, nodes);
    for(int i = 0;i < 20;i++)
        if(nodes[i]->start == id)
            found = nodes[i];
    free(nodes);
    return found;
}

struct idr* idr_setup()
{
    struct idr* idr = (struct idr*)kalloc(sizeof(struct idr), KERN_MEM);
    idr->top = (struct idr_layer*)kalloc(sizeof(struct idr_layer), KERN_MEM);
    idr->layers_allocated = 0;
    idr->id_allocated = 0;
    idr->id_free_cnt = 1; // you get only one
    initlock(&(idr->lock), "idr");
    return idr;
}

void idr_free_layers(struct idr_layer* layer)
{
    for(int i = 0;i < 8;i++)
        if(idr_is_leaf(layer->ary[i]))
            free(layer->ary[i]);
        else
            idr_free_layers(layer->ary[i]); // traverse again, yeey!
    free(layer);
}

void idr_free(struct idr* idr)
{
    idr_free_layers(idr->top);
    idr->id_allocated = 0;
    idr->id_count = 0;
    free(idr);
}

struct idr_node* idr_find_id(struct idr* idr, unsigned long id)
{
    return (struct idr_node*)NULL_PTR;
}

// idr is capable of setting an appropiate resource
void idr_set_resource(struct idr* idr, unsigned long id, void* resource)
{
    struct idr_node* node = idr_find(idr, id);
    node->ptr = resource;
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

void idr_append_id(struct idr* idr, unsigned long id)
{
    if(idr_is_full(idr))
        return; // if it is full then return
    acquire(&(idr->lock));
    idr_add_id(idr->top, id);
    idr->id_count++; // add more IDS
    release(&(idr->lock));
}