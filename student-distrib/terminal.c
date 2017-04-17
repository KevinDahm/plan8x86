#include "terminal.h"
#include "lib.h"
#include "kbd.h"
#include "rtc.h"
#include "system_calls.h"
#include "i8259.h"
#include "page.h"
/* void terminal_init()
 * Decription: Initialzes the terminal operations
 * Input: none
 * Output: none
 * Side Effects: initializes function pointers for system calls
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
 * Input: filename - unused
 * Output: 0 for success
 * Side Effects: None
 */
int32_t terminal_open(const int8_t* filename) {
    return 0;
}

/* void terminal_close(int32_t fd)
 * Decription: Closes the terminal, which currently does nothing
 * Input: fd - unused
 * Output: 0 for success
 * Side Effects: None
 */
int32_t terminal_close(int32_t fd) {
    return 0;
}

/* int32_t stdin_write(int32_t fd, const void* buf, int32_t nbytes)
 * Decription: Fails, don't write to stdin
 * Input: fd - ignored
 *        buf - ignored
 *        nbytes - ignored
 * Output: -1 for bad call
 * Side Effects: none
 */
int32_t stdin_write(int32_t fd, const void* buf, int32_t nbytes){
    return -1;
}

/* int32_t stdout_read(int32_t fd, void* buf, int32_t nbytes)
 * Decription: Fails, don't read from stdout
 * Input: fd - ignored
 *        buf - ignored
 *        nbytes - ignored
 * Output: -1 for bad call
 * Side Effects: none
 */
int32_t stdout_read(int32_t fd, void* buf, int32_t nbytes){
    return -1;
}

/* int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes)
 * Decription: Writes the string buf to the screen
 * Input: fd - ignored
 *        buf - pointer to string to write
 *        nbytes - number of characters to write
 * Output: 0 for success
 * Side Effects: Writes to video memory
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
 * Input: fd - Ignored
 *        buf - pointer to write string
 *        nbytes - number of characters wanted, at most 128
 * Output: Number of valid chars written to buf
 * Side Effects: Writes to buf and video memory, calls command functions
 */
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes) {
    kbd_t k;
    int32_t total = 0;
    int8_t a;
    nbytes = nbytes > KBD_BUFFER_SIZE ? KBD_BUFFER_SIZE : nbytes; //Cap line size at 128
    memset(buf, 0, nbytes);
    // TODO: Arrow keys?
    while (true) { //Keep reading
        if (kbd_read(0, &k, 2)){ //Read a single key
            if(kbd_equal(k, L_KEY) && k.ctrl) { //Reset the screen
                clear();
                set_cursor(0, 0);
                total = 0;
            } else if(kbd_equal(k, BKSP_KEY)) { //Delete the previous character
                if (total > 0) {
                    ((uint8_t*)buf)[total-1] = 0;
                    removec();
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
                ((uint8_t*)buf)[total] = '\t';
                terminal_write(1, buf + total, 1);
                total++;

            } else if(!k.ctrl && (a = kbd_to_ascii(k)) != '\0' && total < nbytes) {
                //not a special character, just write it to the screen
                ((uint8_t*)buf)[total] = a;
                terminal_write(1, buf + total, 1);
                total++;
            }
        }
    }
    return total;
}
