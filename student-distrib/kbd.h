#ifndef KBD_H_
#define KBD_H_

#include "types.h"

typedef struct kbd {
    uint8_t col : 5;
    uint8_t row : 3;
    uint8_t capsLock : 1;
    uint8_t super : 1;
    uint8_t shift : 1;
    uint8_t alt : 1;
    uint8_t ctrl : 1;
    uint8_t padding : 3;
} __attribute__((packed)) kbd_t;

kbd_t kbd_state;

extern void do_irq_0x1();

extern int8_t kbd_to_ascii(kbd_t key);

extern kbd_t get_kbd_state();

#endif
