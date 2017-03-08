#include "idt.h"
#include "i8259.h"
#include "lib.h"

char getScancode() {
    char c = 0;
    do {
        if (inb(0x60) != c) {
            c = inb(0x60);
            if (c > 0) {
                return c;
            }
        }
    } while (1);
}

__attribute__((fastcall)) void do_IRQ(const struct pt_regs* regs) {
    int irq = ~(regs->orig_eax);
    // http://wiki.osdev.org/%228042%22_PS/2_Controller#Translation
    if (irq == 0x21) {
        printf("0x%x\n", getScancode());
    } else if (irq != 0x20) {
        printf("0x%x\n", irq);
    }
    if (irq >= 0x20 && irq < 0x30) {
        send_eoi(irq - 0x20);
    }
}
