/*
 * Implementation of memory functionalitites
 * Functions like malloc, alloca, memcpy, memmove defined here
*/

#ifndef __MEMORY_IMP__
#define __MEMORY_IMP__

#include "../malloc.h"
#include "../alloca.h"
#include "usermode.h"

uint8_t __placeholder[] = {0};

void memset(void* p, char value, size_t len)
{
    char* ptr = (char*)p;
    for(int i = 0;i < len;i++)
        ptr[i] = value;
}

void* malloc(size_t bytes)
{
    return userm_malloc(bytes);
}

void* calloc(size_t num, size_t size)
{
    if(num == NULL || size == NULL)
        return (void*)__placeholder;
    void* p = malloc(num * size);
    memset(p, 0, num * size);
    return p;
}

void free(void* p)
{
    userm_free(p);
}

#endif