#include "kbd.h"
#include "lib.h"
#include "i8259.h"
#include "user_system_calls.h"
#include "system_calls.h"
#include "rtc.h"
#include "terminal.h"

#define SET(s,r,c) case s: kbd_state.row = r; kbd_state.col = c; break
#define BUFFER_SIZE 128
static uint8_t e0_waiting = 0;
static uint8_t kbd_ready = 0;
static uint8_t caps_held = 0;
static uint32_t write_index = 0;
static uint8_t buffer_full = 0;

static kbd_t kbd_buffer[BUFFER_SIZE];

// Current kbd state
kbd_t kbd_state;

int8_t ascii_lookup[][16] = {
    {'\0', 0x1B, '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'},
    {'`', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0x8, '\0', '\0'},
    {0x9, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\\', 0x7F, '\0'},
    {'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '\n', '\0', '\0', '\0', '\0'},
    {'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', '\0', '\0', '\0', '\0', '\0', '\0'},
    {' ', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}
};

int8_t ascii_shift_lookup[][16] = {
    {'\0', 0x1B, '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'},
    {'~', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '=', 0x8, '\0', '\0'},
    {0x9, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '|', 0x7F, '\0'},
    {'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '\n', '\0', '\0', '\0', '\0'},
    {'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', '\0', '\0', '\0', '\0', '\0', '\0'},
    {' ', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}
};

void _kbd_do_irq(int dev_id) {
    uint8_t c;
    c = inb(0x60);
    if (c == 0xE0) {
        e0_waiting = 1;
        return;
    }

    uint16_t scanCode = c;

    if (e0_waiting) {
        scanCode = (0xE0 << 8) | c;
    }

    switch (scanCode) {
        // Left/Right Control pressed
    case 0x001D:
    case 0xE01D:
        kbd_state.ctrl = 1;
        kbd_state.row = 0;
        kbd_state.col = 0;
        break;

        // Left/Right Control released
    case 0x009D:
    case 0xE09D:
        kbd_state.ctrl = 0;
        kbd_state.row = 0;
        kbd_state.col = 0;
        break;

        // Left/Right Alt pressed
    case 0x0038:
    case 0xE038:
        kbd_state.row = 0;
        kbd_state.col = 0;
        kbd_state.alt = 1;
        break;

        // Left/Right Alt released
    case 0x00B8:
    case 0xE0B8:
        kbd_state.row = 0;
        kbd_state.col = 0;
        kbd_state.alt = 0;
        break;

        // Left/Right Shift pressed
    case 0x02A:
    case 0x0036:
        kbd_state.row = 0;
        kbd_state.col = 0;
        kbd_state.shift = 1;
        break;

        // Left/Right Shift released
    case 0x00AA:
    case 0x00B6:
        kbd_state.row = 0;
        kbd_state.col = 0;
        kbd_state.shift = 0;
        break;

        // Left/Right GUI/Super pressed
    case 0xE05B:
    case 0xE05C:
        kbd_state.row = 0;
        kbd_state.col = 0;
        kbd_state.super = 1;
        break;

        // Left/Right GUI/Super released
    case 0xE0DC:
    case 0xE0DB:
        kbd_state.row = 0;
        kbd_state.col = 0;
        kbd_state.super = 0;
        break;

        // Capslock pressed
    case 0x003A:
        if (!caps_held)
            kbd_state.capsLock ^= 1;
        caps_held = 1;
        break;
        // Capslock released
    case 0x00BA: caps_held = 0; break;

        // Defines cases for all keys other than the above
        SET(0x0001, 0, 1);   // ESC
        SET(0x003B, 0, 2);   // F1
        SET(0x003C, 0, 3);   // F2
        SET(0x003D, 0, 4);   // F3
        SET(0x003E, 0, 5);   // F4
        SET(0x003F, 0, 6);   // F5
        SET(0x0040, 0, 7);   // F6
        SET(0x0041, 0, 8);   // F7
        SET(0x0042, 0, 9);   // F8
        SET(0x0043, 0, 10);   // F9
        SET(0x0044, 0, 11);  // F10
        SET(0x0057, 0, 12);  // F11
        SET(0x0058, 0, 13);  // F12
        SET(0x0029, 1, 0);   // ` ~
        SET(0x0002, 1, 1);   // 1 !
        SET(0x0003, 1, 2);   // 2 @
        SET(0x0004, 1, 3);   // 3 #
        SET(0x0005, 1, 4);   // 4 $
        SET(0x0006, 1, 5);   // 5 %
        SET(0x0007, 1, 6);   // 6 ^
        SET(0x0008, 1, 7);   // 7 &
        SET(0x0009, 1, 8);   // 8 *
        SET(0x000A, 1, 9);   // 9 (
        SET(0x000B, 1, 10);  // 0 )
        SET(0x000C, 1, 11);  // - _
        SET(0x000D, 1, 12);  // = +
        SET(0x000E, 1, 13);  // BKSP
        SET(0xE052, 1, 14);  // INSERT
        SET(0xE047, 1, 15);  // HOME
        SET(0xE049, 1, 16);  // PGUP
        SET(0x000F, 2, 0);   // TAB
        SET(0x0010, 2, 1);   // Q
        SET(0x0011, 2, 2);   // W
        SET(0x0012, 2, 3);   // E
        SET(0x0013, 2, 4);   // R
        SET(0x0014, 2, 5);   // T
        SET(0x0015, 2, 6);   // Y
        SET(0x0016, 2, 7);   // U
        SET(0x0017, 2, 8);   // I
        SET(0x0018, 2, 9);   // O
        SET(0x0019, 2, 10);  // P
        SET(0x001A, 2, 11);  // [ {
        SET(0x001B, 2, 12);  // ] }
        SET(0x002B, 2, 13);  // \ |
        SET(0xE053, 2, 14);  // DEL
        SET(0xE04F, 2, 15);  // END
        SET(0xE051, 2, 16);  // PGDN
        SET(0x001E, 3, 0);   // A
        SET(0x001F, 3, 1);   // S
        SET(0x0020, 3, 2);   // D
        SET(0x0021, 3, 3);   // F
        SET(0x0022, 3, 4);   // G
        SET(0x0023, 3, 5);   // H
        SET(0x0024, 3, 6);   // J
        SET(0x0025, 3, 7);   // K
        SET(0x0026, 3, 8);   // L
        SET(0x0027, 3, 9);   // ; :
        SET(0x0028, 3, 10);  // ' "
        SET(0x001C, 3, 11);  // ENTER
        SET(0x002C, 4, 0);   // Z
        SET(0x002D, 4, 1);   // X
        SET(0x002E, 4, 2);   // C
        SET(0x002F, 4, 3);   // V
        SET(0x0030, 4, 4);   // B
        SET(0x0031, 4, 5);   // N
        SET(0x0032, 4, 6);   // M
        SET(0x0033, 4, 7);   // , <
        SET(0x0034, 4, 8);   // . >
        SET(0x0035, 4, 9);   // / ?
        SET(0xE048, 4, 10);  // UP
        SET(0x0039, 5, 0);   // SPACE
        SET(0xE04B, 5, 1);   // LEFT
        SET(0xE050, 5, 2);   // DOWN
        SET(0xE04D, 5, 3);   // RIGHT

        // If we don't recognize the key, clear kbd_state (no key is being pressed/one was just released)
    default:
        kbd_state.row = 0;
        kbd_state.col = 0;
        break;
    }
    // If buffer isn't full and a key is pressed
    if(!buffer_full && kbd_state.state & 0xFF) {
        // Write key to buffer
        kbd_buffer[write_index] = kbd_state;
        // Increment write_index
        write_index++;

        // If buffer full, disable writing
        if (write_index == BUFFER_SIZE)
            buffer_full = 1;
    }
    e0_waiting = 0;
    kbd_ready = 1;
}

void kbd_init(irqaction* keyboard_handler) {
    keyboard_handler->handle = _kbd_do_irq;
    keyboard_handler->dev_id = 0x21;
    keyboard_handler->next = NULL;

    kbd_ops.open = kbd_open;
    kbd_ops.close = kbd_close;
    kbd_ops.read = kbd_read;
    kbd_ops.write = kbd_write;

    irq_desc[0x1] = keyboard_handler;
    enable_irq(1);
}

int32_t kbd_open(const int8_t* filename) {
    return 0;
}

int32_t kbd_close(int32_t fd) {
    return 0;
}

int32_t kbd_write(int32_t fd, const void* buf, int32_t nbytes) {
    return -1;
}

int32_t kbd_read(int32_t fd, void* buf, int32_t nbytes) {
    uint32_t i = 0;
    uint32_t j = 0;
    kbd_t k;
    uint8_t a;
    uint32_t index = 0;
    nbytes = nbytes > 128 ? 128 : nbytes;

    while (i < nbytes) {
        if (write_index != 0) {
            j = 0;
            while(1) {
                k = kbd_buffer[j];
                if(kbd_equal(k, F4_KEY)) {
                    clear();
                    set_cursor(0, 0);
                    uint32_t rtc_freq = BASE_RTC_FREQ;
                    uint32_t dummy;
                    uint32_t fd_rtc = sys_open("/dev/rtc");
                    enable_irq(8);
                    while(!kbd_equal(k, F5_KEY)){
                        sys_read(fd_rtc, &dummy, 4);
                        printf("1");
                        if (write_index != 0 && !kbd_equal(k, F5_KEY)) {
                            j = 0;
                            while(1) {
                                k = kbd_buffer[j];
                                if(kbd_equal(k, F4_KEY)){
                                    rtc_freq <<= 1;
                                    if(rtc_freq > RTC_MAX_FREQ)
                                        rtc_freq = BASE_RTC_FREQ;
                                    sys_write(fd_rtc, &rtc_freq, 4);
                                }else if(kbd_equal(k, F5_KEY)){
                                    break;
                                }
                                j++;
                                cli();
                                if(i == nbytes || j == write_index) {
                                    write_index = 0;
                                    sti();
                                    break;
                                } else {
                                    sti();
                                }
                            }
                        }
                    }
                    disable_irq(8);
                    sys_close(fd_rtc);
                }else if(kbd_equal(k,F1_KEY)) {
                    //LIST FILES
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
                        sys_stat(fd,&data, sizeof(fstat_t));
                        printf("File Type: %d    File Size: %dB\n", data.type, data.size);
                    }
                    sys_close(fd);
                } else if(kbd_equal(k, F2_KEY)) {
                    clear();
                    set_cursor(0,0);
                    int8_t file[] = "frame0.txt";
                    int32_t fd2 = sys_open(file);
                    fstat_t data;
                    sys_stat(fd2, &data, sizeof(fstat_t));
                    uint32_t size = data.size + 1;
                    int8_t text[size];
                    sys_read(fd2, text, size-1);
                    text[size - 1] = '\0';
                    printf(text);
                    printf("\nFile Name: %s", file);
                    sys_close(fd2);
                }
                else if(kbd_equal(k, F3_KEY)){
                    uint32_t it = 0;
                    clear();
                    set_cursor(0,0);
                    int8_t dir[] = {"."};
                    int32_t fd = sys_open(dir);
                    int8_t name[33];
                    while((sys_read(fd, name, 33) != 0) && it < index) {it++;}
                    if(it < index){
                        index = 0;
                    }else{
                        index++;
                    }
                    name[32] = 0;
                    uint32_t fd2 = sys_open(name);
                    fstat_t data;
                    if(sys_stat(fd2, &data, sizeof(fstat_t)) == 0){
                        int8_t text[data.size];
                        sys_read(fd2, text, data.size);
                        terminal_write(1, text, data.size);
                    }
                    printf("\nFile Name: %s", name);
                    sys_close(fd2);
                    sys_close(fd);
                }else if(kbd_equal(k, L_KEY) && k.ctrl) {
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
                    write_index = 0;
                    return i;
                } else if(kbd_equal(k,TAB_KEY)){
                    a = ' ';
                    ((uint8_t*)buf)[i] = '\t';
                    i++;
                    putc(a);
                    putc(a);
                    putc(a);
                    putc(a);
                } else if((a = kbd_to_ascii(k)) != '\0') {
                    ((uint8_t*)buf)[i] = a;
                    i++;
                    putc(a);
                }

                j++;
                cli();
                if(i == nbytes || j == write_index) {
                    write_index = 0;
                    sti();
                    break;
                } else {
                    sti();
                }
            }
        }
    }
    return i;
}

int8_t kbd_to_ascii(kbd_t key) {
    int8_t out;
    if (!key.shift) {
        out = ascii_lookup[key.row][key.col];
    } else {
        out = ascii_shift_lookup[key.row][key.col];
    }

    if (key.capsLock) {
        if (out >= 'A' && out <= 'Z') {
            out -= 'A'-'a';
        } else if (out >= 'a' && out <= 'z') {
            out -= 'a'-'A';
        } else {}
    }


    return out;
}

void _kbd_print_ascii(kbd_t key) {
    int8_t k = kbd_to_ascii(key);
    if (k) {
        printf("%c", k);
    }
}

kbd_t kbd_poll() {
    return kbd_state;
}

kbd_t kbd_poll_echo() {
    kbd_t key = kbd_state;
    _kbd_print_ascii(key);
    return key;
}

kbd_t kbd_get_echo() {
    while (!kbd_ready) {asm volatile ("hlt");}
    kbd_ready = 0;
    _kbd_print_ascii(kbd_state);
    return kbd_state;
}

kbd_t kbd_get() {
    while (!kbd_ready) {asm volatile ("hlt");}
    kbd_ready = 0;
    return kbd_state;
}

uint8_t kbd_equal(kbd_t x, uint8_t y) {
    return (x.state & 0xFF) == y;
}
