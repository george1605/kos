#include "mem.h"
#include "fs.h"

void* find_mem()
{
    for(void* ptr = VM_NUM; ptr <= DRIV_MEM; ptr += 4096)
        if(*(size_t*)ptr != 0xDEADBEEF)
            return ptr;
}

void fil_map(fileptr_t ptr, void* mem, uint16_t off)
{
    void* x = find_mem();
    struct buf* p = ata_read(0, ptr / 512);
    int n = ptr % 512 + off;
    memcpy(x, p->data + n, 512 - n);
}

// relative pointers
struct PtrNode
{
    char info;
    struct PtrNode* left;
    struct PtrNode* right;
};

void* ptr_offset(struct PtrNode p)
{
    return (void*)(p->left - &p);
}