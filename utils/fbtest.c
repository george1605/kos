#define _USE_KOSSTD_
#include "../stdlib/imp/stdio.h"

int main()
{
    FILE* fp = devopen("/dev/fb");
    uint32_t* fb = NULL;
    devctl(fp, FBGETP, &fb);
    for(int i = 0;i < 100;i++)
        for(int j = 0;j < 100;j++)
            fb[j * 100 + i] = (i << 16) | (j << 8);
    devclose(fp);
}