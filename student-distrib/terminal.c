#include "terminal.h"
#include "lib.h"
#include "kbd.h"
#include "rtc.h"
#include "system_calls.h"
#include "user_system_calls.h"
#include "i8259.h"

/* void terminal_init()
 * Decription: Initialzes the terminal operations
 * input: none
 * output: none
 * Side effects: initializes function pointers for system calls
 */
void terminal_init() {
    //setup stdin
    stdin_ops.open = terminal_open;
    stdin_ops.close = terminal_close;
    stdin_ops.write = stdin_write;
    stdin_ops.read = terminal_read;

    //setup stdout
    stdout_ops.open = terminal_open;
    stdout_ops.close = terminal_close;
    stdout_ops.write = terminal_write;
    stdout_ops.read = stdout_read;
}

/* void terminal_open(const int8_t* filename)
 * Decription: Opens the terminal, which currently does nothing
 * input: filename - unused
 * output: 0 for success
 * Side effects: None
 */
int32_t terminal_open(const int8_t* filename) {
    return 0;
}

/* void terminal_close(int32_t fd)
 * Decription: Closes the terminal, which currently does nothing
 * input: fd - unused
 * output: 0 for success
 * Side effects: None
 */
int32_t terminal_close(int32_t fd) {
    return 0;
}

/* int32_t stdin_write(int32_t fd, const void* buf, int32_t nbytes)
 * Decription: Fails, don't write to stdin
 * input: fd - ignored
 *        buf - ignored
 *        nbytes - ignored
 * output: -1 for bad call
 * Side effects: none
 */
int32_t stdin_write(int32_t fd, const void* buf, int32_t nbytes){
    return -1;
}

/* int32_t stdout_read(int32_t fd, void* buf, int32_t nbytes)
 * Decription: Fails, don't read from stdout
 * input: fd - ignored
 *        buf - ignored
 *        nbytes - ignored
 * output: -1 for bad call
 * Side effects: none
 */
int32_t stdout_read(int32_t fd, void* buf, int32_t nbytes){
    return -1;
}

/* int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes)
 * Decription: Writes the string buf to the screen
 * input: fd - ignored
 *        buf - pointer to string to write
 *        nbytes - number of characters to write
 * output: 0 for success
 * Side effects: Writes to video memory
 */
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
    uint32_t fd_rtc = open((uint8_t*)"/dev/rtc");
    kbd_t k;
    write(fd_rtc, &rtc_freq, 4);
    enable_irq(8);
    while(1){
        read(fd_rtc, &dummy, 4);
        printf("1");
        if (kbd_read(0, &k, 2)) {
            if(kbd_equal(k, F4_KEY)){
                rtc_freq <<= 1;
                if(rtc_freq > RTC_MAX_FREQ)
                    rtc_freq = BASE_RTC_FREQ;
                write(fd_rtc, &rtc_freq, 4);
            }else if(kbd_equal(k, F5_KEY)){
                disable_irq(8);
                close(fd_rtc);
                return;
            }
        }
    }
}

void list_files(){
    uint32_t it;
    clear();
    set_cursor(0,0);
    uint8_t dir[] = ".";
    int32_t fd = open(dir);
    int8_t text[33];
    fstat_t data;
    while((read(fd, text, 33) != 0)) {
        text[32] = 0;
        printf("File Name: %s",text);
        for(it = 0; it < 35 - strlen(text); it++)
            printf(" ");
        stat(fd, &data, sizeof(fstat_t));
        printf("File Type: %d    File Size: %dB\n", data.type, data.size);
    }
    close(fd);
}

void list_file_name(uint8_t * name){
    clear();
    set_cursor(0,0);
    int32_t fd = open(name);
    fstat_t data;
    stat(fd, &data, sizeof(fstat_t));
    uint32_t size = data.size + 1;
    int8_t text[size];
    read(fd, text, size-1);
    text[size - 1] = '\0';
    write(1, text, size);
    printf("\nFile Name: %s", name);
    close(fd);
}

int32_t list_file_index(int32_t index){
    uint32_t it = 0;
    clear();
    set_cursor(0,0);
    uint8_t dir[] = ".";
    int32_t fd = open(dir);
    uint8_t name[33];
    int8_t test[33];
    while((read(fd, name, 33) != 0) && it < index) {it++;}
    if(read(fd, test, 33) == 0)
        index = 0;
    else
        index++;
    name[32] = 0;
    uint32_t fd2 = open(name);
    fstat_t data;
    if(stat(fd2, &data, sizeof(fstat_t)) == 0){
        int8_t text[data.size];
        read(fd2, text, data.size);
        write(1, text, data.size);
    }
    printf("\nFile Name: %s\n", name);
    close(fd2);
    close(fd);
    return index;
}

/* int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes)
 * Decription: Reads from the keyboard, displaying text and returning a line
 *             to the user
 * input: fd - Ignored
 *        buf - pointer to write string
 *        nbytes - number of characters wanted, at most 128
 * output: Number of valid chars written to buf
 * Side effects: Writes to buf and video memory, calls command functions
 */
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes) {
    kbd_t k;
    uint32_t i = 0;
    uint32_t index = 0;
    uint8_t name[] = "frame0.txt";
    int8_t a;
    nbytes = nbytes > 128 ? 128 : nbytes; //Cap line size at 128
    while (i < nbytes) { //Keep reading until nbytes
        if (kbd_read(0, &k, 2)){ //Read a single key
            if(kbd_equal(k, F1_KEY)){ //List files for checkpoint 2
                list_files();
            } else if(kbd_equal(k, F2_KEY)){ //List file by name for checkpoint 2
                list_file_name(name);
            } else if(kbd_equal(k, F3_KEY)){ //List file by index for checkpoint 2
                index = list_file_index(index);
            } else if(kbd_equal(k, F4_KEY)){ //Test the rtc for checkpoint 2
                rtc_test();
            } else if(kbd_equal(k, L_KEY) && k.ctrl) { //Reset the screen
                clear();
                set_cursor(0, 0);
                i = 0;
            } else if(kbd_equal(k, BKSP_KEY)) { //Delete the previous character
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
            } else if(kbd_equal(k, ENTER)) { //Write a newline and return
                a = '\n';
                ((uint8_t*)buf)[i] = a;
                i++;
                putc(a);
                return i;
            } else if(kbd_equal(k,TAB_KEY)){// We write a tab as 4 spaces
                a = ' ';
                ((uint8_t*)buf)[i] = '\t';
                i++;
                putc(a);
                putc(a);
                putc(a);
                putc(a);
            } else if(!k.ctrl && (a = kbd_to_ascii(k)) != '\0') {
                //not a special character, just write it to the screen
                ((uint8_t*)buf)[i] = a;
                i++;
                putc(a);
            }
        }
    }
    return i;
}
