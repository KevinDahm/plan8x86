#include "idt.h"
#include "i8259.h"
#include "lib.h"
#include "task.h"
#include "system_calls.h"
#include "page.h"


/* hang
 * Description: Puts the computer to sleep in an infinite loop
 * Input:  none
 * Output: none
 * Side Effects: Disables interrupts
 */
void hang() {
    asm volatile("cli; .1: hlt; jmp .1;");
}

/* handle_exception
 * Description: If cur_task is the kernel we print the exception on a blue screen and hang
 *              if the exception came from a user task we print the exception and halt the program with status 256
 * Input:  exc_str - Exception string to print
 *         err_val - Any hex value to be printed after the string, usually the eip register
 * Output: none
 * Side Effects: Either infinitely sleeps or kills the user process
 */
void handle_exception(int8_t *exc_str, uint32_t err_val) {
    if(cur_task == 0) {
        blue_screen();
        set_cursor(34, 11);
        printf("%s\n", exc_str);
        set_cursor(35, 12);
        printf("0x%#x", err_val);
        hang();
    } else {
        printf("\n%s ", exc_str);
        printf("0x%#x\n", err_val);
        SET_SIGNAL(cur_task, SEGFAULT);
        reschedule();
    }
}


void do_debug(const struct pt_regs* regs) { handle_exception("debug", regs->eip); }
void do_nmi(const struct pt_regs* regs) { handle_exception("nmi", regs->eip); }
void do_int3(const struct pt_regs* regs) { handle_exception("int3", regs->eip); }
void do_overflow(const struct pt_regs* regs) { handle_exception("overflow", regs->eip); }
void do_bounds(const struct pt_regs* regs) { handle_exception("bounds", regs->eip); }
void do_invalid_op(const struct pt_regs* regs) { handle_exception("invalid_op", regs->eip); }
void do_device_not_available(const struct pt_regs* regs) { handle_exception("device_not_available", regs->eip); }
void do_double_fault(const struct pt_regs* regs, uint32_t error) { handle_exception("double fau", regs->eip); }
void do_coprocessor_segment_overrun(const struct pt_regs* regs) { handle_exception("coprocessor_segment_overrun", regs->eip); }
void do_invalid_TSS(const struct pt_regs* regs, uint32_t error) { handle_exception("invalid_TSS", regs->eip); }
void do_segment_not_present(const struct pt_regs* regs, uint32_t error) { handle_exception("segment_not_present", regs->eip); }
void do_stack_segment(const struct pt_regs* regs, uint32_t error) { handle_exception("stack_segment", regs->eip); }
void do_general_protection(const struct pt_regs* regs, uint32_t error) { handle_exception("general_protection", regs->eip); }
void do_coprocessor_error(const struct pt_regs* regs) { handle_exception("coprocessor_error", regs->eip); }
void do_alignment_check(const struct pt_regs* regs, uint32_t error) { handle_exception("alignment_check", regs->eip); }
void do_machine_check(const struct pt_regs* regs) { handle_exception("machine_check", regs->eip); }
void do_simd_coprocessor_error(const struct pt_regs* regs) { handle_exception("simd_coprocessor_error", regs->eip); }

void do_divide_error(const struct pt_regs* regs) {
    if(cur_task == 0) {
        blue_screen();
        set_cursor(34, 11);
        printf("divide error\n");
        set_cursor(35, 12);
        printf("0x%#x", regs->eip);
        hang();
    } else {
        printf("\ndivide error ");
        printf("0x%#x\n", regs->eip);
        SET_SIGNAL(cur_task, DIV_ZERO);
        reschedule();
    }
}
void do_page_fault(const struct pt_regs* regs, uint32_t error) {
    uint32_t cr2;
    asm volatile("movl %%cr2, %0;"
                 : "=r"(cr2)
                 :);

    printf("\nPage fault.\n");
    if (error & 0x1) {
        printf("page-protection violation\n");
    } else {
        printf("non-present page\n");
    }

    if (error & 0x2) {
        printf("page write\n");
    } else {
        printf("page read\n");
    }

    if (error & 0x4) {
        printf("caused while CPL=3\n");
    } else {
        printf("caused while CPL=0\n");
    }

    if (error & 0x8) {
        printf("caused by reading a 1 in a reserved field\n");
    }

    if (error & 0x10) {
        printf("caused by an instruction fetch\n");
    }

    printf("caused by address: 0x%x\n", cr2);
    printf("on EIP: 0x%x\n", regs->eip);
    if (cur_task == 0) {
        hang();
    } else {
        SET_SIGNAL(cur_task, SEGFAULT);
        reschedule();
    }
}

/* do_IRQ
 * Description: Walks the irqaction linked list and executes each handler
 * Input:  regs - The registers saved on the stack when the interrupt occurred
 * Output: none
 * Side Effects: Calls every handler in irq_descs[irq]
 */
__attribute__((fastcall)) void do_IRQ(const struct pt_regs* regs) {
    // TODO: Make this support threading and preempt_count. See UTLK page 213
    int irq = ~(regs->orig_eax);
    irqaction *irq_p = irq_desc[irq];
    while (irq_p) {
        (*irq_p->handle)(irq_p->dev_id);
        irq_p = irq_p->next;
    }
    send_eoi(irq);
}
