
#ifndef KBD_H_
#define KBD_H_

#include "types.h"
#include "idt.h"
#include "filesystem.h"

// Struct for current kdb state
typedef struct kbd {
    union {
        uint16_t state;
        struct {
            uint8_t col : 5;
            uint8_t row : 3;
            uint8_t capsLock : 1;
            uint8_t super : 1;
            uint8_t shift : 1;
            uint8_t alt : 1;
            uint8_t ctrl : 1;
            uint8_t padding : 3;
        } __attribute__((packed));
    };
} kbd_t;

file_ops_t kbd_ops;

// Initialize the KBD handler
extern void kbd_init(irqaction* keyboard_handler);

// Waits until a key is pressed then prints and returns it
extern kbd_t kbd_get_echo();

// Waits until key is predded then returns it
extern kbd_t kbd_get();

// Polls the current kbd_state
extern kbd_t kbd_poll();

// Polls the current kbd_state and prints it
extern kbd_t kbd_poll_echo();

// Compares kbd_t with a bitfield (Ignores capsLock)
extern uint8_t kbd_equal(kbd_t x, uint8_t y);

extern int8_t kbd_to_ascii(kbd_t key);

extern int32_t kbd_open(const int8_t* filename);
extern int32_t kbd_close(int32_t fd);
extern int32_t kbd_read(int32_t fd, void* buf, int32_t nbytes);

#define ESC_KEY 0x01
#define F1_KEY 0x02
#define F2_KEY 0x03
#define F3_KEY 0x04
#define F4_KEY 0x05
#define F5_KEY 0x06
#define F6_KEY 0x07
#define F7_KEY 0x08
#define F8_KEY 0x09
#define F9_KEY 0x0A
#define F10_KEY 0x0B
#define F11_KEY 0x0C
#define F12_KEY 0x0D
#define L_KEY 0x68
#define N_KEY 0x85
#define ENTER 0x6B
#endif
