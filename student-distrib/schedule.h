#ifndef _PIT_H
#define _PIT_H

#include "types.h"
#include "idt.h"

#define NUM_TERM 3
#define TASK_T (tasks[cur_task]->terminal)
#define IS_ACTIVE (active == TASK_T)

extern uint32_t interupt_preempt;

extern uint32_t term_process[NUM_TERM];

extern uint32_t active;

void schedule(int dev_id);
void pit_init(irqaction* pit_handler);

#endif
