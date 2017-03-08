#include "idt.h"
#include "lib.h"

__attribute__((fastcall)) unsigned int do_IRQ(const struct pt_regs* regs) {
    int irq = ~(regs->orig_eax);

    if (irq == 0) {
        printf("test");
    }

    return 1;
}
