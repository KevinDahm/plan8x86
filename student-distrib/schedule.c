#include "schedule.h"
#include "idt.h"
#include "task.h"
#include "page.h"
#include "system_calls.h"
#include "lib.h"
#include "x86_desc.h"
#include "i8259.h"

static int act_term = -1;

uint32_t term_process[NUM_TERM] = {0, 0, 0};

uint32_t active = 0;

// TODO: Get rid of this, this is hacky
uint32_t interupt_preempt = 0;

void schedule(int dev_id){
    uint32_t ebp;
    asm volatile(" \n\
    movl %%ebp, %0 \n"
                 : "=r"(ebp)
                 :);
    tasks[cur_task]->regs.ebp = ebp;

    //   printf(" %x ", ebp);
    if (interupt_preempt) {
        act_term = active;
        interupt_preempt = 0;
    } else {
        act_term = (act_term+1)%NUM_TERM;
    }
    cur_task = term_process[act_term];
    switch_page_directory(cur_task);
    if(cur_task == 0){
        term_process[act_term] = act_term + 1;
        //       printf("if%d", act_term);
        tasks[0]->terminal = act_term;
        send_eoi(0);
        sys_execute((uint8_t*)"shell");
    }

    //  printf("Hey%d", cur_task);
    ebp = tasks[cur_task]->regs.ebp;


    asm volatile(" \n\
movl %0, %%ebp \n"
                 :
                 : "r"(ebp));

    tss.esp0 = tasks[cur_task]->kernel_esp;

}
void pit_init(irqaction* pit_handler){
//setup the handler, on line 8
    pit_handler->handle = schedule;
    pit_handler->dev_id = 0x20;
    pit_handler->next = NULL;
    irq_desc[0x0] = pit_handler;

    // 00110100b - Magic
    outb(0x34, 0x43);
    outb(0, 0x40);
    outb(0, 0x40);
}


