#include "terminal.h"
#include "lib.h"
#include "kbd.h"
#include "rtc.h"
#include "system_calls.h"
#include "i8259.h"

void terminal_init() {
    stdin_ops.open = terminal_open;
    stdin_ops.close = terminal_close;
    stdin_ops.write = stdin_write;
    stdin_ops.read = terminal_read;

    stdout_ops.open = terminal_open;
    stdout_ops.close = terminal_close;
    stdout_ops.write = terminal_write;
    stdout_ops.read = stdout_read;
}

int32_t terminal_open(const int8_t* filename) {
    return 0;
}

int32_t terminal_close(int32_t fd) {
    return 0;
}
int32_t stdin_write(int32_t fd, const void* buf, int32_t nbytes){
    return -1;
}
int32_t stdout_read(int32_t fd, void* buf, int32_t nbytes){
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


void rtc_test(){
    clear();
    set_cursor(0, 0);
    uint32_t rtc_freq = BASE_RTC_FREQ;
    uint32_t dummy;
    uint32_t fd_rtc = sys_open("/dev/rtc");
    kbd_t k;
    sys_write(fd_rtc, &rtc_freq, 4);
    enable_irq(8);
    while(1){
        sys_read(fd_rtc, &dummy, 4);
        printf("1");
        if (kbd_read(0, &k, 2)) {
            if(kbd_equal(k, F4_KEY)){
                rtc_freq <<= 1;
                if(rtc_freq > RTC_MAX_FREQ)
                    rtc_freq = BASE_RTC_FREQ;
                sys_write(fd_rtc, &rtc_freq, 4);
            }else if(kbd_equal(k, F5_KEY)){
                disable_irq(8);
                sys_close(fd_rtc);
                return;
            }
        }
    }
}

void list_files(){
    uint32_t it;
    clear();
    set_cursor(0,0);
    int8_t dir[] = ".";
    int32_t fd = sys_open(dir);
    int8_t text[33];
    fstat_t data;
    while((sys_read(fd, text, 33) != 0)) {
        text[32] = 0;
        printf("File Name: %s",text);
        for(it = 0; it < 35 - strlen(text); it++)
            printf(" ");
        sys_stat(fd, &data, sizeof(fstat_t));
        printf("File Type: %d    File Size: %dB\n", data.type, data.size);
    }
    sys_close(fd);
}

void list_file_name(int8_t * name){
    clear();
    set_cursor(0,0);
    int32_t fd = sys_open(name);
    fstat_t data;
    sys_stat(fd, &data, sizeof(fstat_t));
    uint32_t size = data.size + 1;
    int8_t text[size];
    sys_read(fd, text, size-1);
    text[size - 1] = '\0';
    printf("\nFile Name: %s", name);
    sys_write(1, text, size);
    sys_close(fd);
}

int32_t list_file_index(int32_t index){
    uint32_t it = 0;
    clear();
    set_cursor(0,0);
    int8_t dir[] = {"."};
    int32_t fd = sys_open(dir);
    int8_t name[33];
    int8_t test[33];
    while((sys_read(fd, name, 33) != 0) && it < index) {it++;}
    if(sys_read(fd, test, 33) == 0)
        index = 0;
    else
        index++;
    name[32] = 0;
    uint32_t fd2 = sys_open(name);
    fstat_t data;
    if(sys_stat(fd2, &data, sizeof(fstat_t)) == 0){
        int8_t text[data.size];
        sys_read(fd2, text, data.size);
        sys_write(1, text, data.size);
    }
    printf("\nFile Name: %s\n", name);
    sys_close(fd2);
    sys_close(fd);
    return index;
}
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes) {
    kbd_t k;
    uint32_t i = 0;
    uint32_t index = 0;
    int8_t name[] = "frame0.txt";
    int8_t a;
    nbytes = nbytes > 128 ? 128 : nbytes;
    while (i < nbytes) {
        if (kbd_read(0, &k, 2)){
            if(kbd_equal(k, F1_KEY)){
                list_files();
            } else if(kbd_equal(k, F2_KEY)){
                list_file_name(name);
            } else if(kbd_equal(k, F3_KEY)){
                index = list_file_index(index);
            } else if(kbd_equal(k, F4_KEY)){
                rtc_test();
            } else if(kbd_equal(k, L_KEY) && k.ctrl) {
                clear();
                set_cursor(0, 0);
                i = 0;
            } else if(kbd_equal(k, BKSP_KEY)) {
                if(i > 0) {
                    if(((uint8_t*)buf)[--i]=='\t') {
                        removec();
                        removec();
                        removec();
                    }
                    removec();
                }
            } else if(kbd_equal(k,ESC_KEY)){
                /* TODO: ESCAPE FUNCTIONALITY */
            } else if(kbd_equal(k, ENTER)) {
                a = '\n';
                ((uint8_t*)buf)[i] = a;
                i++;
                putc(a);
                return i;
            } else if(kbd_equal(k,TAB_KEY)){
                a = ' ';
                ((uint8_t*)buf)[i] = '\t';
                i++;
                putc(a);
                putc(a);
                putc(a);
                putc(a);
            } else if(!k.ctrl && (a = kbd_to_ascii(k)) != '\0') {
                ((uint8_t*)buf)[i] = a;
                i++;
                putc(a);
            }

        }
    }
    return i;
}
