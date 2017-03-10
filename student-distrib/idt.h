#ifndef IDT_H
#define IDT_H

#define NR_IRQS 16

struct pt_regs {
    int ebx;
    int ecx;
    int edx;
    int esi;
    int edi;
    int ebp;
    int eax;
    int xds;
    int xes;
    int xfs;
    int orig_eax;
    int eip;
    int xcs;
    int eflags;
    int esp;
    int xss;
};

struct irqaction {
    void (*handle)(int);
    int dev_id;
    struct irqaction* next;
};

typedef struct irqaction irqaction;

irqaction *irq_desc[NR_IRQS];

__attribute__((fastcall)) extern void do_IRQ(const struct pt_regs* regs);

void irq_0x0_handler(int dev_id);
void irq_0x1_handler(int dev_id);

#endif
