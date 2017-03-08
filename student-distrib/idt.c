#include "idt.h"
#include "lib.h"

__attribute__((fastcall)) void do_IRQ(const struct pt_regs* regs) {
    int irq = ~(regs->orig_eax);
}
