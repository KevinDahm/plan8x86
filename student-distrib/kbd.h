
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
//Convert key struct to ascii, including shift and caps lock
extern int8_t kbd_to_ascii(kbd_t key);
//Open keyboard - does nothing
extern int32_t kbd_open(const int8_t* filename);
//Close keyboard - does nothing
extern int32_t kbd_close(int32_t fd);
//Return key from internal buffer
extern int32_t kbd_read(int32_t fd, void* buf, int32_t nbytes);
//Write to keyboard - fails
extern int32_t kbd_write(int32_t fd, const void* buf, int32_t nbytes);

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
#define A_KEY 0x60
#define B_KEY 0x84
#define C_KEY 0x82
#define D_KEY 0x62
#define E_KEY 0x43
#define F_KEY 0x63
#define G_KEY 0x64
#define H_KEY 0x65
#define I_KEY 0x48
#define J_KEY 0x66
#define K_KEY 0x67
#define L_KEY 0x68
#define M_KEY 0x86
#define N_KEY 0x85
#define O_KEY 0x49
#define P_KEY 0x4A
#define Q_KEY 0x41
#define R_KEY 0x44
#define S_KEY 0x61
#define T_KEY 0x45
#define U_KEY 0x47
#define V_KEY 0x83
#define W_KEY 0x42
#define X_KEY 0x81
#define Y_KEY 0x46
#define Z_KEY 0x80
#define ENTER 0x6B
#define BKSP_KEY 0x2D
#define TAB_KEY 0x40
#endif
