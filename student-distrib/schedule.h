#ifndef _PIT_H
#define _PIT_H

#include "types.h"
#include "idt.h"

#define NUM_TERM 3
#define TASK_T (tasks[cur_task]->terminal)
#define IS_ACTIVE (active == TASK_T)

extern bool interupt_preempt;

extern uint32_t term_process[NUM_TERM];

extern uint32_t active;

extern void schedule();
void pit_init();

#endif
