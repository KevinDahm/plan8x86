#include "schedule.h"
#include "idt.h"
#include "task.h"
#include "page.h"
#include "system_calls.h"
#include "lib.h"
#include "x86_desc.h"
#include "i8259.h"
#include "signals.h"

uint32_t term_process[NUM_TERM] = {0, 0, 0};

uint32_t active = 0;

bool interupt_preempt = false;

static int cur_p = 0;

#define PIT_PORT_COMMAND 0x43
#define PIT_PORT_CHANNEL_0 0x40
#define LOW_FREQ_BYTE 0
// ~15ms time slices
#define HIGH_FREQ_BYTE 75

/* void reschedule()
 * Decription: Switches the active process
 * input: none
 * output: none
 * Side effects: writes to the PIT, changes the current task
 */
void reschedule() {
    cli();

    outb(LOW_FREQ_BYTE, PIT_PORT_CHANNEL_0);
    outb(HIGH_FREQ_BYTE, PIT_PORT_CHANNEL_0);

    sti();

    asm volatile("int $0x20;");
}

/* void backup_uesp(hw_context_t *hw_context)
 * Decription: Back up the user stack pointer
 * input: hw_context - pointer to the hw_context to store
 * output: none
 * Side effects: writes the stack pointer to the pcb
 */
void backup_uesp(hw_context_t *hw_context) {
    if (hw_context->iret_context.eip >= TASK_ADDR) {
        tasks[cur_task]->user_esp = hw_context->iret_context.esp;
    }
}

/* void schedule()
 * Decription: PIT irq handler, switches the active process
 * input: none
 * output: none
 * Side effects: switches the active process
 */
void schedule() {
    uint32_t ebp;
    asm volatile("movl %%ebp, %0;" : "=r"(ebp) : );
    tasks[cur_task]->ebp = ebp;

    send_eoi(0);

    if (backup_init_ebp) {
        cur_task = INIT;
    } else if (interupt_preempt) {
        cur_task = term_process[active];
        interupt_preempt = false;
    } else {

        cur_task = INIT;

        // Starting with the task after the last one scheduled look for the
        // first task that is TASK_RUNNING, but skip over INIT.
        // If no task needs to be scheduled, only then do we schedule INIT
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

    tss.esp0 = tasks[cur_task]->kernel_esp;

    ebp = tasks[cur_task]->ebp;
    asm volatile("movl %0, %%ebp;" : : "r"(ebp));

    // GCC compiles this function with no leave call. Thus the ebp becomes useless.
    asm volatile("leave; ret;" : :);
}

/* void pit_init()
 * Decription: Initialzes the PIT
 * input: none
 * output: none
 * Side effects: writes to the PIT
 */
void pit_init(){
    // 00110100b - Magic (sets the pit up for regular interrupts on IRQ0)
    outb(0x34, PIT_PORT_COMMAND);
    outb(LOW_FREQ_BYTE, PIT_PORT_CHANNEL_0);
    outb(HIGH_FREQ_BYTE, PIT_PORT_CHANNEL_0);
}
