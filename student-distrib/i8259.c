/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts
 * are enabled and disabled */
uint8_t master_mask = 0xFF; /* IRQs 0-7 */
uint8_t slave_mask = 0xFF; /* IRQs 8-15 */


/* i8259_init
 * Description: Initializes the 8259 PICs
 * Input:  none
 * Output: none
 * Side Effects: Sets up the PICs
 */
void i8259_init() {
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

/* enable_irq
 * Description: Enable (unmask) the specified IRQ
 * Input:  irq_num - the irq to enable
 * Output: none
 * Side Effects: Enables irq_num on the PIC
 */
void enable_irq(uint32_t irq_num) {
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

/* disable_irq
 * Description: Disable (mask) the specified IRQ
 * Input:  irq_num - the irq to disable
 * Output: none
 * Side Effects: Disables irq_num on the PIC
 */
void disable_irq(uint32_t irq_num) {
    if(irq_num < 8){ //Line on master
        // Enable bit #irq_num
        master_mask |= (1 << irq_num);
        outb(master_mask, MASTER_8259_PORT + 1);
    }else{ //Line on slave
        // Enable bit #irq_num % 8
        slave_mask |= (1 << (irq_num - 8));
        outb(slave_mask, SLAVE_8259_PORT + 1);
    }
}

/* disable_irq
 * Description: Send end-of-interrupt signal for the specified IRQ
 * Input:  irq_num - the irq to send EOI to
 * Output: none
 * Side Effects: Sends EOI to irq_num
 */
void send_eoi(uint32_t irq_num) {
    outb(EOI | irq_num, MASTER_8259_PORT);
    //EOI | irq_num tells the PIC that irq_num was handled
    if(irq_num >= 0x8){
        outb(EOI | (irq_num-8), SLAVE_8259_PORT);
        outb(EOI | 2, MASTER_8259_PORT);
    }
}
