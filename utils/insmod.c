#include "../elf.h"
#include "../stdlib.c"

int main(int argc, char** argv)
{
    if(argc < 2)
    {
        printf("Format: insmod <file>");
        exit(-1);
    }
    userm_insmod(argv[1]);
    exit(0);
}