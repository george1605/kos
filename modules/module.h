#include "../elf.h"
#include "../fs.h"
#define _kmod

struct kmodule {
    void (*onattach)();
    void (*ondettach)();
    int version;
    struct ElfSection* elf;
} *modlist;

void modload(char* fname){
  
}
