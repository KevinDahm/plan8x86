/* kernel.c - the C part of the kernel
 * vim:ts=4 noexpandtab
 */

#include "multiboot.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "debug.h"
#include "entry.h"
#include "idt.h"
#include "kbd.h"
#include "page.h"
#include "rtc.h"
#include "user_system_calls.h"
#include "filesystem.h"
#include "system_calls.h"

/* Macros. */
/* Check if the bit BIT in FLAGS is set. */
#define CHECK_FLAG(flags,bit)   ((flags) & (1 << (bit)))

/* Check if MAGIC is valid and print the Multiboot information structure
   pointed by ADDR. */
void
entry (unsigned long magic, unsigned long addr)
{
    multiboot_info_t *mbi;

    /* Clear the screen. */
    clear();

    /* Am I booted by a Multiboot-compliant boot loader? */
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
    {
        printf ("Invalid magic number: 0x%#x\n", (unsigned) magic);
        return;
    }

    /* Set MBI to the address of the Multiboot information structure. */
    mbi = (multiboot_info_t *) addr;

    /* Print out the flags. */
    printf ("flags = 0x%#x\n", (unsigned) mbi->flags);

    /* Are mem_* valid? */
    if (CHECK_FLAG (mbi->flags, 0))
        printf ("mem_lower = %uKB, mem_upper = %uKB\n",
                (unsigned) mbi->mem_lower, (unsigned) mbi->mem_upper);

    /* Is boot_device valid? */
    if (CHECK_FLAG (mbi->flags, 1))
        printf ("boot_device = 0x%#x\n", (unsigned) mbi->boot_device);

    /* Is the command line passed? */
    if (CHECK_FLAG (mbi->flags, 2))
        printf ("cmdline = %s\n", (char *) mbi->cmdline);

    if (CHECK_FLAG (mbi->flags, 3)) {
        int mod_count = 0;
        int i;
        module_t* mod = (module_t*)mbi->mods_addr;
        file_system_init((void*)mod->mod_start, (void*)mod->mod_end);
        while(mod_count < mbi->mods_count) {
            printf("Module %d loaded at address: 0x%#x\n", mod_count, (unsigned int)mod->mod_start);
            printf("Module %d ends at address: 0x%#x\n", mod_count, (unsigned int)mod->mod_end);
            printf("First few bytes of module:\n");
            for(i = 0; i<16; i++) {
                printf("0x%x ", *((char*)(mod->mod_start+i)));
            }
            printf("\n");
            mod_count++;
            mod++;
        }
    }
    /* Bits 4 and 5 are mutually exclusive! */
    if (CHECK_FLAG (mbi->flags, 4) && CHECK_FLAG (mbi->flags, 5))
    {
        printf ("Both bits 4 and 5 are set.\n");
        return;
    }

    /* Is the section header table of ELF valid? */
    if (CHECK_FLAG (mbi->flags, 5))
    {
        elf_section_header_table_t *elf_sec = &(mbi->elf_sec);

        printf ("elf_sec: num = %u, size = 0x%#x,"
                " addr = 0x%#x, shndx = 0x%#x\n",
                (unsigned) elf_sec->num, (unsigned) elf_sec->size,
                (unsigned) elf_sec->addr, (unsigned) elf_sec->shndx);
    }

    /* Are mmap_* valid? */
    if (CHECK_FLAG (mbi->flags, 6))
    {
        memory_map_t *mmap;

        printf ("mmap_addr = 0x%#x, mmap_length = 0x%x\n",
                (unsigned) mbi->mmap_addr, (unsigned) mbi->mmap_length);
        for (mmap = (memory_map_t *) mbi->mmap_addr;
             (unsigned long) mmap < mbi->mmap_addr + mbi->mmap_length;
             mmap = (memory_map_t *) ((unsigned long) mmap
                                      + mmap->size + sizeof (mmap->size)))
            printf (" size = 0x%x,     base_addr = 0x%#x%#x\n"
                    "     type = 0x%x,  length    = 0x%#x%#x\n",
                    (unsigned) mmap->size,
                    (unsigned) mmap->base_addr_high,
                    (unsigned) mmap->base_addr_low,
                    (unsigned) mmap->type,
                    (unsigned) mmap->length_high,
                    (unsigned) mmap->length_low);
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
        the_tss_desc.dpl            = 0x0;
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

    // Paging setup
    clear_tables();
    create_entries();
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
    irqaction rtc_handler;
    rtc_init(&rtc_handler);
    set_intr_gate(0x28, irq_0x8);
    //Initialize keyboard and enable it's interrupts
    irqaction keyboard_handler;
    kbd_init(&keyboard_handler);
    set_intr_gate(0x21, irq_0x1);

    lidt(idt_desc_ptr);

    clear();
    set_cursor(0, 0);

    /* printf("Enabling Interrupts\n"); */
    sti();

    system_calls_init();
    int fd = open("frame0.txt");
    int8_t text[188];
    read(fd, text, 187);
    text[187] = 0;
    printf("%s", text);

    /* /\* Execute the first program (`shell') ... *\/ */
    /* kbd_t a; */
    /* uint8_t x = 1; */
    /* int* test; */
    /* while(1){ */
    /*     a = kbd_get_echo(); */
    /*     if(kbd_equal(a, F1_KEY)){ */
    /*         if(x & 1){ */
    /*             x &= ~1; */
    /*             enable_irq(8); */
    /*         }else{ */
    /*             x |= 1; */
    /*             disable_irq(8); */
    /*         } */
    /*     }if(kbd_equal(a, L_KEY) && a.ctrl){ */
    /*         clear(); */
    /*         set_cursor(0, 0); */
    /*     }if(kbd_equal(a, N_KEY) && a.ctrl){ */
    /*         test = 0; */
    /*         *test = 5; */
    /*     } */
    /* } */
    /* Spin (nicely, so we don't chew up cycles) */
    asm volatile(".1: hlt; jmp .1;");
}
