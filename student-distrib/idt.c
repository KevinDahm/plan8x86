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
