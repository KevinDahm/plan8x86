#ifndef ENTRY_H
#define ENTRY_H

extern void ignore_int();

extern void execute_shell();

extern void divide_error();
extern void debug();
extern void nmi();
extern void int3();
extern void overflow();
extern void bounds();
extern void invalid_op();
extern void device_not_available();
extern void double_fault();
extern void coprocessor_segment_overrun();
extern void invalid_TSS();
extern void segment_not_present();
extern void stack_segment();
extern void general_protection();
extern void page_fault();
extern void coprocessor_error();
extern void alignment_check();
extern void machine_check();
extern void simd_coprocessor_error();
extern void system_call();

extern void do_divide_error();
extern void do_debug();
extern void do_nmi();
extern void do_int3();
extern void do_overflow();
extern void do_bounds();
extern void do_invalid_op();
extern void do_device_not_available();
extern void do_double_fault();
extern void do_coprocessor_segment_overrun();
extern void do_invalid_TSS();
extern void do_segment_not_present();
extern void do_stack_segment();
extern void do_general_protection();
extern void do_page_fault();
extern void do_coprocessor_error();
extern void do_alignment_check();
extern void do_machine_check();
extern void do_simd_coprocessor_error();

extern void irq_0x0();
extern void irq_0x1();
extern void irq_0x2();
extern void irq_0x3();
extern void irq_0x4();
extern void irq_0x5();
extern void irq_0x6();
extern void irq_0x7();
extern void irq_0x8();
extern void irq_0x9();
extern void irq_0xA();
extern void irq_0xB();
extern void irq_0xC();
extern void irq_0xD();
extern void irq_0xE();
extern void irq_0xF();

#endif
