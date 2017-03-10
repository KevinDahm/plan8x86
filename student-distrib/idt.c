#include "idt.h"
#include "i8259.h"
#include "lib.h"

void blue_screen(const char * str) {
    clear();
    // TODO: blue
    printf(str);
    asm volatile(".1: hlt; jmp .1;");
}

void do_divide_error() {
    blue_screen("divide_error");
}

void do_debug() {
    blue_screen("debug");
}

void do_nmi() {
    blue_screen("nmi");
}

void do_int3() {
    blue_screen("int3");
}

void do_overflow() {
    blue_screen("overflow");
}

void do_bounds() {
    blue_screen("bounds");
}

void do_invalid_op() {
    blue_screen("invalid_op");
}

void do_device_not_available() {
    blue_screen("device_not_available");
}

void do_double_fault() {
    blue_screen("double_fault");
}

void do_coprocessor_segment_overrun() {
    blue_screen("coprocessor_segment_overrun");
}

void do_invalid_TSS() {
    blue_screen("invalid_TSS");
}

void do_segment_not_present() {
    blue_screen("segment_not_present");
}

void do_stack_segment() {
    blue_screen("stack_segment");
}

void do_general_protection() {
    blue_screen("general_protection");
}

void do_page_fault() {
    blue_screen("page_fault");
}

void do_coprocessor_error() {
    blue_screen("coprocessor_error");
}

void do_alignment_check() {
    blue_screen("alignment_check");
}

void do_machine_check() {
    blue_screen("machine_check");
}

void do_simd_coprocessor_error() {
    blue_screen("simd_coprocessor_error");
}

void do_system_call() {
    blue_screen("system_call");
}

// TODO: Make this support threading and preempt_count. See UTLK page 213
__attribute__((fastcall)) void do_IRQ(const struct pt_regs* regs) {
    int irq = ~(regs->orig_eax);
    irqaction *irq_p = irq_desc[irq];
    while (irq_p) {
        (*irq_p->handle)(irq_p->dev_id);
        irq_p = irq_p->next;
    }
    send_eoi(irq + 0x20);
}

void irq_0x0_handler(int dev_id) {
    test_interrupts();
}

void irq_0x1_handler(int dev_id) {
    char c = 0;
    do {
        if (inb(0x60) != c) {
            c = inb(0x60);
            if (c > 0) {
                break;
            }
        }
    } while (1);

    printf("%x", c);
}
