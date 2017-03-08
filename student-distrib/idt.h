#ifndef IDT_H
#define IDT_H

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

__attribute__((fastcall)) extern void do_IRQ(const struct pt_regs* regs);

#endif
