#ifndef __ATOMIC_IMP__
#define __ATOMIC_IMP__
#include "malloc.h"

typedef struct {
    void* data;
    int locked;
} atomic_t;

atomic_t* atomic_create(void* data_handle)
{
    atomic_t* atom = (atomic_t*)malloc(sizeof(atomic_t));
    atom->locked = 0;
    atom->data = data_handle;
}

void atomic_acquire(atomic_t* atomic) {
    asm volatile(
        "1:\n\t"
        "xorl %%eax, %%eax\n\t"    // Clear EAX register
        "xchgl %0, %%eax\n\t"      // Atomic exchange locked with 0
        "testl %%eax, %%eax\n\t"   // Test the value in EAX
        "jnz 1b\n\t"                // If not zero, retry
        : "+m" (atomic->locked)
        :
        : "%eax", "memory"
    );
}

// Atomic release operation using xchg instruction
void atomic_release(atomic_t* atomic) {
    asm volatile(
        "movl $0, %0\n\t"   // Set locked to 0
        : "+m" (atomic->locked)
        :
        : "memory"
    );
}

#endif