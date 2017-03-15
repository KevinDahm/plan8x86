#include "shell.h"
#include "types.h"
#include "system_calls.h"
#include "user_system_calls.h"
#include "filesystem.h"
#include "kbd.h"
#include "lib.h"
#include "idt.h"
#include "i8259.h"

void shell() {
    /* int fd = open("frame0.txt"); */
    /* int8_t text[188]; */
    /* read(fd, text, 187); */
    /* close(fd); */
    /* printf("%s", text); */

    int fd_rtc = open("/dev/rtc");
    int32_t rtc_freq;
    int32_t x = 0;
    int32_t counter;

    int fd_kbd = open("/dev/kbd");
    kbd_t buf[10];
    kbd_t a = {{0}};
    int i;
    int size;

    while(1){
        if(kbd_equal(a, F4_KEY)){
            clear();
            set_cursor(0, 0);
            rtc_freq = 0;
            counter = 0;
            while(!kbd_equal(a, F5_KEY)){
                enable_irq(8);
                read(fd_rtc, &x, 4);
                printf("1");
                if(counter++ > 5){
                    counter = 0;
                    rtc_freq = (rtc_freq+1)%10;
                    write(fd_rtc, &rtc_freq, 4);
                }
                if(read(fd_kbd, &buf, 2))
                    a = buf[0];
            }
            disable_irq(8);
            clear();
            set_cursor(0,0);
        } else if(kbd_equal(a, L_KEY) && a.ctrl){
            clear();
            set_cursor(0, 0);
        } else if(kbd_equal(a, N_KEY) && a.ctrl){
            if((size = read(fd_kbd, &buf, 10))){
                for(i = 0; i < size; i++)
                    if(!(a=buf[i]).ctrl)
                        printf("%c",kbd_to_ascii(buf[i]));
            }
        }

        a = kbd_poll();
        asm volatile ("hlt;");
    }
}
