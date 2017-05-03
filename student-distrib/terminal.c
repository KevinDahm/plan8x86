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
    int32_t i = 0;
    int32_t j;
    int8_t a;
    uint32_t startx, starty, endx, endy;
    nbytes = nbytes > KBD_BUFFER_SIZE ? KBD_BUFFER_SIZE : nbytes; //Cap line size at 128
    memset(buf, 0, nbytes);
    startx = get_cursor_x();
    starty = get_cursor_y();
    while (true) { //Keep reading
        if (kbd_read(0, &k, 2)) { //Read a single key
            if(kbd_to_ascii(k) == 'l' && k.ctrl) { //Reset the screen
                uint8_t temp[startx];
                for (j = 0; j < startx; j++) {
                    get_video(temp, 0, starty, startx);
                }
                uint32_t x = get_cursor_x();
                uint32_t y = get_cursor_y();
                clear();
                set_cursor(0, 0);
                terminal_write(1, temp, startx);
                terminal_write(1, buf, total);
                set_cursor(x, y - starty);
                endy -= starty;
                starty = 0;
            } else if (k.ctrl && kbd_to_ascii(k) == 'k') {
                total = i;
                pclear(get_cursor_x(), get_cursor_y(), endx, endy);
                endx = get_cursor_x();
                endy = get_cursor_y();
            } else if (k.ctrl && kbd_to_ascii(k) == 'a') {
                i = 0;
                set_cursor(startx, starty);
            } else if (k.ctrl && kbd_to_ascii(k) == 'e') {
                i = total;
                set_cursor(endx, endy);
            } else if (kbd_equal(k, BKSP_KEY)) { //Delete the previous character
                if (i > 0) {
                    a = ((uint8_t*)buf)[i-1];
                    memmove(buf + i - 1, buf + i, total-i);
                    if(a == '\t'){
                        removec(4);
                    }else{
                        removec(1);
                        a = ' ';
                    }
                    total--;
                    i--;
                    uint32_t x = get_cursor_x();
                    uint32_t y = get_cursor_y();
                    terminal_write(1, buf + i, total - i);
                    endx = get_cursor_x();
                    endy = get_cursor_y();
                    putc(a); //clear leftover garbage
                    set_cursor(x, y);

                }
            } else if (kbd_equal(k, DEL_KEY) || (kbd_to_ascii(k) == 'd' && k.ctrl)) {
                if (i < total) {
                    //move right
                    int32_t h = ((uint8_t*)buf)[i] == '\t' ? 4 : 1;
                    move_hor(h);
                    i++;
                    //delete
                    a = ((uint8_t*)buf)[i-1];
                    memmove(buf + i - 1, buf + i, total-i);
                    if (a == '\t') {
                        removec(4);
                    } else {
                        removec(1);
                        a = ' ';
                    }
                    total--;
                    i--;
                    uint32_t x = get_cursor_x();
                    uint32_t y = get_cursor_y();
                    terminal_write(1, buf + i, total - i);
                    endx = get_cursor_x();
                    endy = get_cursor_y();
                    putc(a);//clear up garbage
                    set_cursor(x, y);
                }
            } else if (kbd_equal(k, ESC_KEY)) {
                total = 0;
                i = 0;
                pclear(startx, starty, endx, endy);
                set_cursor(startx, starty);
                endx = startx;
                endy = starty;
            } else if (kbd_equal(k, ENTER)) { //Write a newline and return
                if (total != nbytes) {
                    a = '\n';
                    ((uint8_t*)buf)[total] = a;
                    total++;
                    putc(a);
                }
                return total;
            } else if (kbd_equal(k,TAB_KEY) && total < nbytes) { // We write a tab as 4 spaces
                memmove(buf + i + 1, buf + i, total-i);
                ((uint8_t*)buf)[i] = '\t';
                total++;
                uint32_t x = get_cursor_x();
                uint32_t y = get_cursor_y();
                terminal_write(1, buf + i, total-i);
                endx = get_cursor_x();
                endy = get_cursor_y();
                set_cursor(x, y);
                move_hor(4);
                i++;
            } else if (kbd_equal(k, RIGHT_KEY) || (kbd_to_ascii(k) == 'f' && k.ctrl)) {
                if(total > i) {
                    int32_t x = ((uint8_t*)buf)[i] == '\t' ? 4 : 1;
                    move_hor(x);
                    i++;
                }
            } else if (kbd_equal(k, LEFT_KEY) || (kbd_to_ascii(k) == 'b' && k.ctrl)) {
                if(i > 0) {
                    int32_t x = ((uint8_t*)buf)[i-1] == '\t' ? 4 : 1;
                    move_hor(-x);
                    i--;
                }
            } else if (!k.ctrl && (a = kbd_to_ascii(k)) != '\0' && total < nbytes) {
                //not a special character, just write it to the screen
                memmove(buf + i + 1, buf + i, total-i);
                ((uint8_t*)buf)[i] = a;
                total++;
                uint32_t x = get_cursor_x();
                uint32_t y = get_cursor_y();
                terminal_write(1, buf + i, total-i);
                endx = get_cursor_x();
                endy = get_cursor_y();
                set_cursor(x, y);
                move_hor(1);
                i++;
            }
        }
    }
    return total;
}
