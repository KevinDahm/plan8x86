#ifndef _PIT_H
#define _PIT_H

#include "types.h"

#define NUM_TERM 3
#define TASK_T (tasks[cur_task]->terminal)
#define IS_ACTIVE (active == TASK_T)

extern bool interupt_preempt;

extern uint32_t term_process[NUM_TERM];

extern uint32_t active;

typedef struct {
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t esp;
    uint32_t ds;
} iret_context_t;

typedef struct {
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t ebp;
    uint32_t eax;
    uint32_t ds;
    uint32_t es;
    uint32_t fs;
    uint32_t irq_exc;
    uint32_t err_code;
    iret_context_t iret_context;
} hw_context_t;

extern void backup_uesp(hw_context_t *hw_contex);
extern void reschedule();
extern void schedule(hw_context_t *hw_context);
void pit_init();

#endif
