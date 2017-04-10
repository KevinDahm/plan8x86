#include "rtc.h"
#include "idt.h"
#include "lib.h"
#include "i8259.h"
static void do_rtc_irq(int dev_id);
static int8_t rtc_flag;
static uint8_t num_open;

/* void rtc_init(irqaction* rtc_handler)
 * Decription: Initialzes the rtc and it's irqaction struct for use
 * input: rtc_handler - pointer to irqaction struct for the rtc
 * output: none
 * Side effects: Enables a PIC line, modifies the rtc_handler, writes to
 * rtc registers, and sets up system call function pointers
 */
void rtc_init(irqaction* rtc_handler){
//set control register A
    uint32_t x = BASE_RTC_FREQ;
    rtc_write(0, &x, 4);
//set control register B
    outb(CHOOSE_RTC_B, RTC_PORT);
    uint8_t prev = inb(RTC_PORT + 1);
    outb((prev | RTC_CMD_B), RTC_PORT + 1);

//setup the handler, on line 8
    rtc_handler->handle = do_rtc_irq;
    rtc_handler->dev_id = 0x28;
    rtc_handler->next = NULL;
    irq_desc[0x8] = rtc_handler;

//setup the system call functions
    rtc_ops.open = rtc_open;
    rtc_ops.close = rtc_close;
    rtc_ops.read = rtc_read;
    rtc_ops.write = rtc_write;
    rtc_ops.stat = rtc_stat;

//enable the interrupt
    enable_irq(2);
}

/* int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes)
 * Decription: Writes the frequency in buf to the rtc
 * input: fd - ignored
 *        buf - pointer to frequency as uint32_t, should be a power of 2
 *        nbytes - should always be 4
 * output: -1 for invalid input, 0 for success
 * Side effects: Changes the rtc frequency
 */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes){
    if(nbytes != 4)
        return -1;
    uint32_t freq =  *((uint32_t*)buf);
    if(freq < 2 || freq > RTC_MAX_FREQ){//Check for out of bounds parameters
        return -1;
    }
    uint32_t logf = 0;
    while((!(freq & 1)) && logf < 10){//find log(freq)
        freq = freq >> 1;
        logf++;
    }
    if(freq != 1){//check if freq was a power of 2
        return -1;
    }
    //disable interrupts while writing to the rtc
    uint32_t flags;
    cli_and_save(flags);

    //write log(freq) to the rtc
    outb(CHOOSE_RTC_A, RTC_PORT);
    outb((16-logf) | RTC_CMD_A, RTC_PORT+1);

    restore_flags(flags);
    return 0;
}

/* int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes)
 * Decription: Gives RTC frequency and stalls for interrupt
 * input: fd - Ignored
 *        buf - pointer to write frequency
 *        nbytes - should always be 4
 * output: -1 for invalid input, 0 for success
 * Side effects: Writes to buf, loops until RTC interrupt
 */
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes){
    if(nbytes != 4){
        return -1;
    }
    //disable interrupts while writing to the rtc
    uint32_t flags;
    cli_and_save(flags);

    //Get the frequency
    outb(CHOOSE_RTC_A, RTC_PORT);
    uint8_t freq = inb(RTC_PORT+1);

    restore_flags(flags);

    //Translate the frequency and write it to the buf
    *((uint32_t*)buf) = 1 << (16-(freq & 0xF));

    //loop until RTC interrupt
    do{
        rtc_flag = 1;
        asm volatile("hlt");
    }while(rtc_flag);

    return 0;
}

/* void rtc_open(const int8_t* filename)
 * Decription: Opens the rtc, which currently does nothing
 * input: filename - unused, as it should be the rtc
 * output: 0 for success
 * Side effects: None
 */
int32_t rtc_open(const int8_t* filename){
    enable_irq(8);
    num_open++;
    return 0;
}

/* void rtc_close(int32_t fd)
 * Decription: Closes the rtc, which currently does nothing
 * input: fd - unused
 * output: 0 for success
 * Side effects: None
 */
int32_t rtc_close(int32_t fd){
    num_open--;
    if (!num_open) {
        disable_irq(8);
    }
    return 0;
}

/* void rtc_stat(int32_t fd, void* buf, int32_t nbytes)
 * Decription: Gives stats for the rtc, which currently does nothing
 * input: fd - unused
 *        buf - unused
 *        nbytes - unused
 * output: 0 for success
 * Side effects: None
 */
int32_t rtc_stat(int32_t fd, void* buf, int32_t nbytes) {
    return 0;
}


/* void do_rtc_irq(int dev_id)
 * Decription: Standard rtc handler
 * input: dev_id - currently unused
 * output: none
 * Side effects: Writes to video memory and the RTC
 */
void do_rtc_irq(int dev_id) {

    //Reset rtc_flag
    rtc_flag = 0;
    //read port C to acknowledge the interrupt
    outb(CHOOSE_RTC_C, RTC_PORT);
    inb(RTC_PORT + 1);
}
