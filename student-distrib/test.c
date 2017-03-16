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

#define KBD_BUF_SIZE 128

void test() {


    int fd_rtc = open((uint8_t*)"/dev/rtc");


    int fd_kbd = open((uint8_t*)"/dev/kbd");
    kbd_t buf[KBD_BUF_SIZE];
    static kbd_t a = {{0}};
    uint32_t i,j;
    uint32_t size;

    while(1){
        if((size = read(fd_kbd, &buf, KBD_BUF_SIZE))){
            for(i = 0; i < size; i++)
                a=buf[i];
            if(kbd_equal(a, F4_KEY)){
                rtc_test(fd_kbd, fd_rtc);
            } else if(kbd_equal(a, L_KEY) && a.ctrl){
                clear();
                set_cursor(0, 0);
            } else if(kbd_equal(a, F1_KEY)){
                clear();
                set_cursor(0,0);
                uint8_t* dir = (uint8_t*)".";
                printf("PRINTING DIR: <%s>\n",dir);
                int fd = open(dir);
                int8_t text[33];
                fstat_t data;
                while((read(fd, text, 33) != 0)) {
                    text[32] = 0;
                    printf("File Name: %s",text);
                    for(j = 0; j < 35 - strlen(text); j++)
                        printf(" ");
                    stat(fd,&data, sizeof(fstat_t));
                    printf("File Type: %d    File Size: %dB\n", data.type, data.size);
                }
                close(fd);
            } else if((a.state & 0xFF) != 0) {
                printf("%c",kbd_to_ascii(a));
            }
        }
        asm volatile ("hlt;");
    }
}
