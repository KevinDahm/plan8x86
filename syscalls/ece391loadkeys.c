#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

int main() {
    uint8_t buf[128];
    ece391_getargs(buf, 128);

    if (!ece391_strncmp(buf, (uint8_t*)"qwerty", ece391_strlen((uint8_t*)"qwerty"))) {
        ece391_loadkeys(QWERTY);
    } else if (!ece391_strncmp(buf, (uint8_t*)"dvorak", ece391_strlen((uint8_t*)"dvorak"))) {
        ece391_loadkeys(DVORAK);
    } else {
        ece391_fdputs(1, (uint8_t*)"Did not recognize keyboard layout");
        return 1;
    }

    return 0;
}
