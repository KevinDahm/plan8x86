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

#define BUF_SIZE 128

void test() {
    int fd_kbd = open((uint8_t*)"/dev/kbd");
    kbd_t buf[BUF_SIZE];

    while(1){
        printf("READ STARTING\n");
        read(fd_kbd, buf, BUF_SIZE);
        printf("READ FINISHED\n");
    }
}
