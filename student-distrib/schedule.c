#include "schedule.h"
#include "idt.h"
#include "task.h"
#include "page.h"
#include "system_calls.h"
#include "lib.h"
#include "x86_desc.h"
#include "i8259.h"

uint32_t term_process[NUM_TERM] = {0, 0, 0};

uint32_t active = 0;

// TODO: Get rid of this, this is hacky
uint32_t interupt_preempt = 0;

static int cur_p = 0;

void schedule(uint32_t esp) {
    uint32_t ebp;
    asm volatile(" \n\
    movl %%ebp, %0 \n"
                 : "=r"(ebp)
                 :);
    tasks[cur_task]->regs.ebp = ebp;

    int32_t do_tss = 0;

    // schedule entered from user-land
    if (tasks[cur_task]->kernel_esp - esp != 12) {
        tasks[cur_task]->kernel_esp = ebp - 4;
        do_tss = 1;
    } else {
        int x = 10;
    }

    if (interupt_preempt) {
        cur_task = term_process[active];
        interupt_preempt = 0;
    } else {

        cur_task = 0;

        int i;
        for (i = 0; i < (NUM_TASKS - 1); i++) {
            uint8_t task_i = ((cur_p + i) % (NUM_TASKS - 1)) + 1;
            if (tasks[task_i]->status == TASK_RUNNING) {
                cur_task = task_i;
                cur_p = task_i;
                break;
            }
        }
    }

    switch_page_directory(cur_task);

    if (do_tss) {
        tss.esp0 = tasks[cur_task]->kernel_esp;
    }

    ebp = tasks[cur_task]->regs.ebp;
    asm volatile(" \n\
    movl %0, %%ebp \n"
                 :
                 : "r"(ebp));

    send_eoi(0);
    // GCC compiles this function with no leave call. Thus the ebp becomes useless.
    asm volatile("leave; ret;" : :);
}

void pit_init(){
    // 00110100b - Magic
    outb(0x34, 0x43);
    outb(0, 0x40);
    outb(0, 0x40);
}


