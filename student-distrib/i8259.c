/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts
 * are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7 */
uint8_t slave_mask; /* IRQs 8-15 */

/* Initialize the 8259 PIC */
void
i8259_init(void)
{
    outb(0xff, MASTER_8259_PORT + 1);//Clear interrupts on both PICs
    outb(0xff, SLAVE_8259_PORT + 1);

    outb(ICW1, MASTER_8259_PORT);//init code
    outb(ICW2_MASTER, MASTER_8259_PORT + 1); //Mapped to 0x20-0x27
    outb(ICW3_MASTER, MASTER_8259_PORT + 1); //Slave on line IR2
    outb(ICW4, MASTER_8259_PORT + 1); //Normal EOI

    outb(ICW1, SLAVE_8259_PORT); //init code
    outb(ICW2_SLAVE, SLAVE_8259_PORT + 1); //Mapped to 0x28-0x2F
    outb(ICW3_SLAVE, SLAVE_8259_PORT + 1); //Connected to Master's IR2
    outb(ICW4, SLAVE_8259_PORT + 1); //Normal EOI

    //Reset interrupts to previous state
    outb(master_mask, MASTER_8259_PORT + 1);
    outb(slave_mask, SLAVE_8259_PORT + 1);
}

/* Enable (unmask) the specified IRQ */
void
enable_irq(uint32_t irq_num)
{
    if(irq_num < 8){ //Line on master
        //Disable bit #irq_num
        master_mask |= 1 << irq_num;
        outb(master_mask, MASTER_8259_PORT + 1);
    }else{ //Line on slave
        //Disable bit #irq_num % 8
        slave_mask |= 1 << (irq_num - 8);
        outb(slave_mask, SLAVE_8259_PORT + 1);
    }
}

/* Disable (mask) the specified IRQ */
void
disable_irq(uint32_t irq_num)
{
    if(irq_num < 8){ //Line on master
        //Disable bit #irq_num
        master_mask &= ~(1 << irq_num);
        outb(master_mask, MASTER_8259_PORT + 1);
    }else{ //Line on slave
        //Disable bit #irq_num % 8
        slave_mask &= ~(1 << (irq_num - 8));
        outb(slave_mask, SLAVE_8259_PORT + 1);
    }
}

/* Send end-of-interrupt signal for the specified IRQ */
void
send_eoi(uint32_t irq_num)
{
    //EOI | irq_num tells the PIC that irq_num was handled
    if(irq_num < 8){
        outb(EOI | irq_num, MASTER_8259_PORT);
    }else{
        outb(EOI | (irq_num-8), SLAVE_8259_PORT);
    }
}
