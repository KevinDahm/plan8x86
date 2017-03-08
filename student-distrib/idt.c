#include "idt.h"
#include "i8259.h"
#include "lib.h"

__attribute__((fastcall)) void do_IRQ(const struct pt_regs* regs) {
    int irq = ~(regs->orig_eax);
    printf("0x%x\n", irq);
    if (irq >= 0x20 && irq < 0x30) {
        send_eoi(irq - 0x20);
    }
}
