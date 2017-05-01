#ifndef SIGNALS_H_
#define SIGNALS_H_

#include "types.h"
#include "schedule.h"

//check for signals for the current task
void check_for_signals(hw_context_t *hw_context);

//handles signals if no signal handler has been set
void handle_default_signal(int32_t signal);

//executes signal handlers
void handle_signals(hw_context_t *hw_context);

#endif
