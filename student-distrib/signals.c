#include "signals.h"
#include "task.h"
#include "system_calls.h"

void handle_default_signal(int32_t signal) {
    CLEAR_SIGNAL(cur_task, signal);
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

void handle_signals() {
    int8_t signal;
    for (signal = 0; signal < NUM_SIGNALS; signal++) {
        if (SIGNAL_SET(cur_task, signal)) {
            if (signal_handlers[cur_task][signal] == NULL) {
                handle_default_signal(signal);
            } else {
                // Jump to user code
            }
        }
    }
}
