#include "terminal.h"
#include "lib.h"

void terminal_init() {
    terminal_ops.open = terminal_open;
    terminal_ops.close = terminal_close;
    terminal_ops.write = terminal_write;
    terminal_ops.read = terminal_read;
}

int32_t terminal_open(const int8_t* filename) {
    return 0;
}

int32_t terminal_close(int32_t fd) {
    return 0;
}

int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes) {
    return -1;
}

int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes) {
    register int32_t index = 0;
    while(index < nbytes) {
        putc(((int8_t*)buf)[index]);
        index++;
    }
    return 0;
}
