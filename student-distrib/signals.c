#include "signals.h"
#include "task.h"
#include "system_calls.h"
#include "lib.h"
#include "x86_desc.h"

void check_for_signals(hw_context_t *hw_context) {
    uint32_t task;
    for (task = 1; task < NUM_TASKS; task++) {
        if (tasks[task]->pending_signals != 0) {
            cur_task = task;
            handle_signals(hw_context, task);
        }
    }
}

void handle_default_signal(int32_t signal) {
    switch (signal) {
    case DIV_ZERO:
    case SEGFAULT:
    case INTERRUPT:
        sys_halt(256);
    case ALARM:
    case USER1:
    default:
        return;
    }
}

void handle_signals(hw_context_t *hw_context, uint32_t task) {
    uint8_t signal;
    if (tasks[task]->signal_mask == false) {
        for (signal = 0; signal < NUM_SIGNALS; signal++) {
            if (SIGNAL_SET(task, signal)) {
                CLEAR_SIGNAL(task, signal);
                if (signal_handlers[task][signal] == NULL) {
                    handle_default_signal(signal);
                } else {
                    tasks[task]->signal_mask = true;
                    uint8_t *uesp = (uint8_t *)tasks[task]->user_esp - 4;
                    // Put the following assembly on the user stack:
                    // pushl $0xA, %eax
                    // int $0x80
                    *uesp = 0x00; uesp--;
                    *uesp = 0x80; uesp--;
                    *uesp = 0xCD; uesp--;
                    *uesp = 0x00; uesp--;

                    *uesp = 0x00; uesp--;
                    *uesp = 0x00; uesp--;
                    *uesp = 0x0A; uesp--;
                    *uesp = 0xB8;

                    uint32_t return_addr = (uint32_t)uesp;
                    uesp -= 1;

                    uesp -= sizeof(hw_context_t);
                    memcpy(uesp, hw_context, sizeof(hw_context_t));
                    tasks[task]->sig_hw_context = (hw_context_t *)uesp;

                    uesp -= 4;
                    *(uint32_t *)uesp = signal;
                    uesp -= 4;
                    *(uint32_t *)uesp = return_addr;

                    asm volatile("                             \n\
                    movw $" str(USER_DS) ", %%ax               \n\
                    movw %%ax, %%ds                            \n\
                    movw %%ax, %%es                            \n\
                    movw %%ax, %%fs                            \n\
                    movw %%ax, %%gs                            \n\
                                                               \n\
                    pushl $" str(USER_DS) "                    \n\
                    pushl %1                                   \n\
                    pushf                                      \n\
                    popl %%eax                                 \n\
                    orl $0x200, %%eax                          \n\
                    pushl %%eax                                \n\
                    pushl $" str(USER_CS) "                    \n\
                    pushl %0                                   \n\
                    iret                                       \n\
                    "
                                 :
                                 : "b"(signal_handlers[task][signal]), "c"(uesp));
                }
            }
        }
    }
}
