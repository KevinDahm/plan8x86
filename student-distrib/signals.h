#ifndef SIGNALS_H_
#define SIGNALS_H_

#include "types.h"
#include "schedule.h"

void check_for_signals(hw_context_t *hw_context);
void handle_default_signal(int32_t signal);
void handle_signals(hw_context_t *hw_context);

#endif
