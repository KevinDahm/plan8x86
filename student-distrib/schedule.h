#ifndef _PIT_H
#define _PIT_H
#define NUM_TERM 3

#include "types.h"
#include "idt.h"

extern uint32_t term_process[NUM_TERM];
void schedule(int dev_id);
void pit_init(irqaction* pit_handler);
#endif
