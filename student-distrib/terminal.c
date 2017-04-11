#include "terminal.h"
#include "lib.h"
#include "kbd.h"
#include "rtc.h"
#include "system_calls.h"
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
    int32_t i = 0;
    int32_t total = 0;
    int8_t a;
    int32_t x, y;
    nbytes = nbytes > 128 ? 128 : nbytes; //Cap line size at 128
    memset(buf, 0, nbytes);
    while (1) { //Keep reading
        if (kbd_read(0, &k, 2)){ //Read a single key
            if(kbd_equal(k, L_KEY) && k.ctrl) { //Reset the screen
                clear();
                set_cursor(0, 0);
                i = 0;
                total = 0;
            } else if(kbd_equal(k, BKSP_KEY)) { //Delete the previous character
                if(i > 0) {
                    memmove(buf+i-1, buf+i, total - i);
                    ((uint8_t*)buf)[total-1] = 0;
                    removec();
                    x = get_x();
                    y = get_y();
                    terminal_write(1, buf+i-1, total-i+1);
                    set_cursor(x, y);
                    i--;
                    total--;

                }
            } else if(kbd_equal(k,DEL_KEY)){
                if(total-i != 0){
                    i++;
                    set_cursor(get_x() + 1, get_y());
                    memmove(buf+i-1, buf+i, total - i);
                    ((uint8_t*)buf)[total-1] = 0;
                    removec();
                    x = get_x();
                    y = get_y();
                    terminal_write(1, buf+i-1, total-i+1);
                    set_cursor(x, y);
                    i--;
                    total--;
                }
            } else if(kbd_equal(k, ENTER)) { //Write a newline and return
                if(total != nbytes){
                    a = '\n';
                    ((uint8_t*)buf)[total] = a;
                    total++;
                    putc(a);
                }
                return total;
            } else if(kbd_equal(k,TAB_KEY) && total < nbytes){// We write a tab as 4 spaces
                memmove(buf+i+1, buf+i, total-i);
                ((uint8_t*)buf)[i] = '\t';
                x = get_x();
                y = get_y();
                terminal_write(1, buf+i, total-i+1);
                set_cursor(x+4, y);
                i++;
                total++;

            } else if(kbd_equal(k,LEFT_KEY)){
                if(total-i != total){
                    i--;
                    set_cursor(get_x() - 1, get_y());
                }
            }else if(kbd_equal(k,RIGHT_KEY)){
                if(total-i != 0){
                    i++;
                    set_cursor(get_x() + 1, get_y());
                }
            }else if(!k.ctrl && (a = kbd_to_ascii(k)) != '\0' && total < nbytes) {
                //not a special character, just write it to the screen
                memmove(buf+i+1, buf+i, total - i);
                ((uint8_t*)buf)[i] = a;
                x = get_x();
                y = get_y();
                terminal_write(1, buf+i, total-i+1);
                set_cursor(x+1, y);
                i++;
                total++;

            }
        }
    }
    return total;
}
