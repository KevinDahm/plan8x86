#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

#define SBUFSIZE 33

typedef struct fstat {
    uint8_t type;
    uint32_t size;
}fstat_t;

int main ()
{
    int32_t fd, cnt;
    uint8_t buf[SBUFSIZE];

    uint8_t args[128];
    ece391_getargs(args, 128);

    if (-1 == (fd = ece391_open ((uint8_t*)"."))) {
        ece391_fdputs (1, (uint8_t*)"directory open failed\n");
        return 2;
    }

    if (!ece391_strcmp(args, (uint8_t*)"-l")) {
        uint8_t text[33];
        fstat_t data;
        uint32_t it;
        while((ece391_read(fd, text, 33) != 0)) {
            text[32] = 0;
            printf("File Name: %s",text);
            for(it = 0; it < 35 - ece391_strlen(text); it++)
                printf(" ");
            ece391_stat(fd, &data, sizeof(fstat_t));
            printf((uint8_t*)"File Type: %d    File Size: %dB\n", data.type, data.size);
        }

    } else {
        while (0 != (cnt = ece391_read (fd, buf, SBUFSIZE-1))) {
            if (-1 == cnt) {
                ece391_fdputs (1, (uint8_t*)"directory entry read failed\n");
                return 3;
            }
            buf[cnt] = '\n';
            if (-1 == ece391_write (1, buf, cnt + 1))
                return 3;
        }
    }
    return 0;
}
