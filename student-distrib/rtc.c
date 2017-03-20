#include "rtc.h"
#include "idt.h"
#include "lib.h"
#include "i8259.h"
#include "kbd.h"
#include "user_system_calls.h"
static void do_rtc_irq(int dev_id);
static int8_t rtc_flag;

/* void rtc_init(irqaction* rtc_handler)
 * Decription: Initialzes the rtc and it's irqaction struct for use
 * input: rtc_handler - pointer to irqaction struct for the rtc
 * output: none
 * Side effects: Enables a PIC line, modifies the rtc_handler, and writes to
 * rtc registers
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

    rtc_ops.open = rtc_open;
    rtc_ops.close = rtc_close;
    rtc_ops.read = rtc_read;
    rtc_ops.write = rtc_write;
    rtc_ops.stat = rtc_stat;

//enable the interrupt
    enable_irq(2);
}
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes){
    if(nbytes != 4)
        return -1;
    uint32_t freq =  *((uint32_t*)buf);
    if(freq < 2 || freq > RTC_MAX_FREQ){
        return -1;
    }
    uint32_t logf = 0;
    while((!(freq & 1)) && logf < 10){//check for log(freq)
        freq = freq >> 1;
        logf++;
    }
    if(freq != 1){
        return -1;
    }

    outb(CHOOSE_RTC_A, RTC_PORT);
    outb((16-logf) | RTC_CMD_A, RTC_PORT+1);
    return 0;
}

int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes){
    if(nbytes != 4){
        return -1;
    }
    outb(CHOOSE_RTC_A, RTC_PORT);
    uint32_t freq = inb(RTC_PORT+1);
    *((uint32_t*)buf) = 16-(freq & 0xF);
    do{
        rtc_flag = 1;
        asm volatile("hlt");
    }while(rtc_flag);

    return 0;
}
int32_t rtc_open(const int8_t* filename){
    return 0;
}
int32_t rtc_close(int32_t fd){
    return 0;
}
int32_t rtc_stat(int32_t fd, void* buf, int32_t nbytes) {
    return 0;
}


/* void do_rtc_init(int dev_id)
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
    //This is a fun function
}
