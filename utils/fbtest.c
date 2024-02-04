#define _USE_KOSSTD_
#include "../stdlib/imp/stdio.h"

int main()
{
    FILE* fp = devopen("/dev/fb");
    devctl(fp, )
    devclose(fp);
}