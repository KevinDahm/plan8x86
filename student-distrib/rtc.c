#include "rtc.h"
#include "idt.h"
#include "lib.h"
#include "i8259.h"
static void do_rtc_irq(int dev_id);


/* void rtc_init(irqaction* rtc_handler)
 * Decription: Initialzes the rtc and it's irqaction struct for use
 * input: rtc_handler - pointer to irqaction struct for the rtc
 * output: none
 * Side effects: Enables a PIC line, modifies the rtc_handler, and writes to
 * rtc registers
 */
void rtc_init(irqaction* rtc_handler){
//set control register A
    outb(CHOOSE_RTC_A, RTC_PORT);
    outb(RTC_CMD_A | BASE_RTC_FREQ, RTC_PORT + 1);
//set control register B
    outb(CHOOSE_RTC_B, RTC_PORT);
    uint8_t prev = inb(RTC_PORT + 1);
    outb((prev | RTC_CMD_B), RTC_PORT + 1);

//setup the handler, on line 8
    rtc_handler->handle = do_rtc_irq;
    rtc_handler->dev_id = 0x28;
    rtc_handler->next = NULL;
    irq_desc[0x8] = rtc_handler;

//enable the slave PIC for convenience
    enable_irq(2);
}
/* void do_rtc_init(int dev_id)
 * Decription: Initialzes the rtc and it's irqaction struct for use
 * input: rtc_handler - pointer to irqaction struct for the rtc
 * output: none
 * Side effects: Enables a PIC line, modifies the rtc_handler, and writes to
 * rtc registers
 */
void do_rtc_irq(int dev_id) {
    //read port C to acknowledge the interrupt
    outb(CHOOSE_RTC_C, RTC_PORT);
    inb(RTC_PORT + 1);

    //This is a fun function
    test_interrupts();
}
