#ifndef IDT_H
#define IDT_H

#include "schedule.h"

#define NR_IRQS 16

struct irqaction {
    void (*handle)(int);
    int dev_id;
    struct irqaction* next;
};

typedef struct irqaction irqaction;

irqaction *irq_desc[NR_IRQS];

__attribute__((fastcall)) extern void do_IRQ(hw_context_t* hw_context);

#endif
