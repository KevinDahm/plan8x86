#include "test.h"
#include "types.h"
#include "system_calls.h"
#include "user_system_calls.h"
#include "filesystem.h"
#include "kbd.h"
#include "lib.h"
#include "idt.h"
#include "i8259.h"
#include "rtc.h"
#include "terminal.h"

#define BUF_SIZE 128

void test() {
    //  int fd_kbd = open((uint8_t*)"/dev/kbd");
    kbd_t buf[BUF_SIZE];

    while(1){
        terminal_read(0, buf, BUF_SIZE);
    }
}
