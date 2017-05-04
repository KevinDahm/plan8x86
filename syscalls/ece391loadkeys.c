#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

int main() {
    uint8_t buf[128];
    ece391_getargs(buf, 128);

    int kbd = ece391_open("/dev/kbd");
    uint32_t layout;

    if (!ece391_strncmp(buf, (uint8_t*)"qwerty", ece391_strlen((uint8_t*)"qwerty"))) {
        layout = QWERTY;
        ece391_write(kbd, &layout, 4);
    } else if (!ece391_strncmp(buf, (uint8_t*)"dvorak", ece391_strlen((uint8_t*)"dvorak"))) {
        layout = DVORAK;
        ece391_write(kbd, &layout, 4);
    } else {
        ece391_fdputs(1, (uint8_t*)"Did not recognize keyboard layout");
        return 1;
    }

    return 0;
}
