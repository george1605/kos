/*
 * Simple Test Lib that contains Optionals(Took from C++)
*/
#pragma once
#include "../lib.c"
#include "../fs.h"
#include "../smem.h"
#define ERR_IS_FATAL 1
#define ERR_WARNING  2
#define VAL_COMPARE 0
#define STR_COMPARE 1
#define MMAP_COMPARE 2
#define OPTIONAL_UNUSED (-1)
#define OPTIONAL_NONE OPTIONAL(OPTIONAL_UNUSED)

typedef struct _opt {
    void* value;
    uint8_t is_none;
    void*(*value_or)(struct _opt*, void*);
    void*(*value_or_error)(struct _opt*, char*, int);
} optional;

void* __value_or(optional* _this, void* _default)
{
    if(_this->is_none)
        return _default;
    return _this->value;
}

void* __value_or_error(optional* _this, char* err_name, int erropts)
{
    if(!_this->is_none)
        return _this->value;
    printf("Optional Error: %s\n", err_name);
    if(erropts == ERR_IS_FATAL)
        raise(-1);
    return NULL_PTR;
}

static void optional_setup(optional* _this)
{
    _this->value_or = __value_or;
    _this->value_or_error = __value_or_error;
}

optional OPTIONAL(void* value)
{
    optional temp;
    optional_setup(&temp);
    if(value == (void*)OPTIONAL_UNUSED)
        return (optional){.is_none = 1};
    return (optional){.value = value};
}

uint8_t expect(void* expr, void* value, int comp)
{
    switch(comp)
    {
    case VAL_COMPARE:
        return expr == value;
        break;
    case STR_COMPARE:
        return strcmp((char*)expr, (char*)value);
        break;
    case MMAP_COMPARE:
        return (get_phys(expr) == value);
    }
}