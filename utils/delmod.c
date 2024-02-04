#include "../elf.h"
#include "../stdlib.c"

int main(int argc, char** argv)
{
    if(argc < 2)
    {
        printf("Format: delmod <file>");
        exit(-1);
    }
    userm_delmod(argv[1]);
    exit(0);
}