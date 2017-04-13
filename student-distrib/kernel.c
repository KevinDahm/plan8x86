/* kernel.c - the C part of the kernel
 * vim:ts=4 noexpandtab
 */

#include "entry.h"
#include "idt.h"
#include "i8259.h"
#include "lib.h"
#include "kbd.h"
#include "multiboot.h"
#include "page.h"
#include "rtc.h"
#include "system_calls.h"
#include "terminal.h"
#include "task.h"
#include "schedule.h"
#include "x86_desc.h"
/* Macros. */
/* Check if the bit BIT in FLAGS is set. */
#define CHECK_FLAG(flags,bit)   ((flags) & (1 << (bit)))

// NOTE: These cannot be declared on the stack because the kernel stack is destroyed when user processes start
irqaction keyboard_handler;
irqaction rtc_handler;
irqaction pit_handler;

int8_t shell_str[] = "shell";


void entry (unsigned long magic, unsigned long addr) {
    multiboot_info_t *mbi;

    clear();

    /* Am I booted by a Multiboot-compliant boot loader? */
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
    {
        printf ("Invalid magic number: 0x%#x\n", (unsigned) magic);
        return;
    }

    /* Set MBI to the address of the Multiboot information structure. */
    mbi = (multiboot_info_t *) addr;

    // Store pointers to the filesystem
    if (CHECK_FLAG (mbi->flags, 3)) {
        module_t* mod = (module_t*)mbi->mods_addr;
        file_system_init((void*)mod->mod_start, (void*)mod->mod_end);
    }
    /* Bits 4 and 5 are mutually exclusive! */
    if (CHECK_FLAG (mbi->flags, 4) && CHECK_FLAG (mbi->flags, 5))
    {
        printf ("Both bits 4 and 5 are set.\n");
        return;
    }

    /* Construct an LDT entry in the GDT */
    {
        seg_desc_t the_ldt_desc;
        the_ldt_desc.granularity    = 0;
        the_ldt_desc.opsize         = 1;
        the_ldt_desc.reserved       = 0;
        the_ldt_desc.avail          = 0;
        the_ldt_desc.present        = 1;
        the_ldt_desc.dpl            = 0x0;
        the_ldt_desc.sys            = 0;
        the_ldt_desc.type           = 0x2;

        SET_LDT_PARAMS(the_ldt_desc, &ldt, ldt_size);
        ldt_desc_ptr = the_ldt_desc;
        lldt(KERNEL_LDT);
    }

    /* Construct a TSS entry in the GDT */
    {
        seg_desc_t the_tss_desc;
        the_tss_desc.granularity    = 0;
        the_tss_desc.opsize         = 0;
        the_tss_desc.reserved       = 0;
        the_tss_desc.avail          = 0;
        the_tss_desc.seg_lim_19_16  = TSS_SIZE & 0x000F0000;
        the_tss_desc.present        = 1;
        the_tss_desc.dpl            = 0x3;
        the_tss_desc.sys            = 0;
        the_tss_desc.type           = 0x9;
        the_tss_desc.seg_lim_15_00  = TSS_SIZE & 0x0000FFFF;

        SET_TSS_PARAMS(the_tss_desc, &tss, tss_size);

        tss_desc_ptr = the_tss_desc;

        tss.ldt_segment_selector = KERNEL_LDT;
        tss.ss0 = KERNEL_DS;
        tss.esp0 = 0x800000;
        ltr(KERNEL_TSS);
    }

    // Paging and tasks structure setup
    create_init();
    switch_page_directory(0);
    init_paging();

    /* Init the PIC */
    i8259_init();

    /* Initialize the IDT */
    int i;
    // If an interrupt is generated that we haven't setup complain
    for (i = 0; i < NUM_VEC; i++) {
        set_intr_gate(i, ignore_int);
    }

    // Exceptions
    set_trap_gate(0, divide_error);
    set_trap_gate(1, debug);
    set_intr_gate(2, nmi);
    set_system_intr_gate(3, int3);
    set_system_gate(4, overflow);
    set_system_gate(5, bounds);
    set_trap_gate(6, invalid_op);
    set_trap_gate(7, device_not_available);
    set_trap_gate(8, double_fault);
    set_trap_gate(9, coprocessor_segment_overrun);
    set_trap_gate(10, invalid_TSS);
    set_trap_gate(11, segment_not_present);
    set_trap_gate(12, stack_segment);
    set_trap_gate(13, general_protection);
    set_intr_gate(14, page_fault);
    set_trap_gate(16, coprocessor_error);
    set_trap_gate(17, alignment_check);
    set_trap_gate(18, machine_check);
    set_trap_gate(19, simd_coprocessor_error);
    set_system_gate(0x80, system_call);

    // Initialize RTC, does not enable the interrupt
    rtc_init(&rtc_handler);
    set_intr_gate(0x28, irq_0x8);
    //Initialize keyboard and enable it's interrupts
    kbd_init(&keyboard_handler);
    set_intr_gate(0x21, irq_0x1);

    pit_init(&pit_handler);
    set_intr_gate(0x20, irq_0x0);

    lidt(idt_desc_ptr);

    clear();
    set_cursor(0, 0);

    sti();

    terminal_init();
    system_calls_init();

    /* Execute the first program (`shell') ... */
    enable_irq(0);

    tasks[0]->terminal = 0;
    asm volatile("movl $2, %%eax; movl %0, %%ebx; movl $0, %%ecx; movl $0, %%edx; int $0x80;"
        :
        : "b"(shell_str));
    tasks[0]->terminal = 1;
    asm volatile("movl $2, %%eax; movl %0, %%ebx; movl $0, %%ecx; movl $0, %%edx; int $0x80;"
                 :
                 : "b"(shell_str));
    tasks[0]->terminal = 2;
    asm volatile("movl $2, %%eax; movl %0, %%ebx; movl $0, %%ecx; movl $0, %%edx; int $0x80;"
                 :
                 : "b"(shell_str));
    tasks[0]->terminal = 0;

    printf("INIT INIT INIT\n");

    asm volatile (".1: hlt; jmp .1;");
}
