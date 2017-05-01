#include "rtc.h"
#include "idt.h"
#include "lib.h"
#include "i8259.h"
static void do_rtc_irq(int dev_id);
static uint8_t num_open;
static uint32_t rtc_freq;
static uint32_t sys_time = 0;

/* void rtc_init(irqaction* rtc_handler)
 * Decription: Initialzes the rtc and it's irqaction struct for use
 * input: rtc_handler - pointer to irqaction struct for the rtc
 * output: none
 * Side effects: Enables a PIC line, modifies the rtc_handler, writes to
 * rtc registers, and sets up system call function pointers
 */
void rtc_init(irqaction* rtc_handler){

    //set control register A
    uint32_t flags;
    cli_and_save(flags);

    // write log(freq) to the rtc
    outb(CHOOSE_RTC_A, RTC_PORT);
    outb((16 - BASE_RTC_LOG) | RTC_CMD_A, RTC_PORT+1);

    restore_flags(flags);
    rtc_freq = MAX_RTC_FREQ >> BASE_RTC_LOG;
    //initialize system time rate
    tasks[INIT]->rtc_base = MAX_RTC_FREQ;

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
    enable_irq(8);
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

    // Check for out of bounds parameters
    if(freq < 2 || freq > MAX_RTC_FREQ) {
        return -1;
    }

    // find log(freq)
    uint32_t logf = 0;
    while (!(freq & 1)) {
        freq >>= 1;
        logf++;
    }

    // check if freq was a power of 2
    if (freq != 1) {
        return -1;
    }
    tasks[cur_task]->rtc_base = MAX_RTC_FREQ >> logf;
    if(tasks[cur_task]->rtc_base < rtc_freq){
        rtc_freq = tasks[cur_task]->rtc_base;
        // disable interrupts while writing to the rtc
        uint32_t flags;
        cli_and_save(flags);

        // write log(freq) to the rtc
        outb(CHOOSE_RTC_A, RTC_PORT);
        outb((16 - logf) | RTC_CMD_A, RTC_PORT+1);

        restore_flags(flags);
    }

    return 0;
}

/* int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes)
 * Decription: Gives RTC frequency and stalls for interrupt
 * input: fd - ignored
 *        buf - pointer to number of RTC interrupts since function call
 *        nbytes - size of buf, should be at least 4
 * output: -1 for invalid input, 0 for success
 * Side effects: Writes to buf, sleeps until RTC interrupt
 */
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes){
    if(nbytes < 4) {
        return -1;
    }
    // loop until RTC interrupt
    tasks[cur_task]->rtc_counter = tasks[cur_task]->rtc_base;

    while (tasks[cur_task]->rtc_counter > 0) {
        reschedule();
    }
    *((uint32_t*)buf) = 1 + (-tasks[cur_task]->rtc_counter)/tasks[cur_task]->rtc_base;
    return 0;
}

/* int32_t rtc_open(const int8_t* filename)
 * Decription: Opens the rtc, which currently does nothing
 * input: filename - unused, as it should be the rtc
 * output: 0 for success
 * Side effects: None
 */
int32_t rtc_open(const int8_t* filename){
    num_open++;
    return 0;
}

/* int32_t rtc_close(int32_t fd)
 * Decription: Closes the rtc, which currently does nothing
 * input: fd - unused
 * output: 0 for success
 * Side effects: None
 */
int32_t rtc_close(int32_t fd){
    num_open--;
    return 0;
}

/* int32_t rtc_stat(int32_t fd, void* buf, int32_t nbytes)
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

/* void update_time(bool reset)
 * Decription: Updates the system time
 * input: reset - Whether to reset the hardware RTC frequency to the base rate
 * output: none
 * Side effects: Updates internal system time
 */
void update_time(bool reset){
    sys_time++;
    if(sys_time%10 == 0){
        uint32_t task;
        for (task = 1; task < NUM_TASKS; task++) {
            SET_SIGNAL(task, ALARM);
        }
    }
    if(reset){ //reset the rtc to the default rate if possible
        uint32_t flags;
        cli_and_save(flags);

        // write log(freq) to the rtc
        outb(CHOOSE_RTC_A, RTC_PORT);
        outb((16 - BASE_RTC_LOG) | RTC_CMD_A, RTC_PORT+1);// 15 = 16 - log(BASE_RTC_FREQ)

        restore_flags(flags);
    }
    tasks[INIT]->rtc_counter = tasks[INIT]->rtc_base;
}

uint32_t get_time(){
    return sys_time;
}

/* void do_rtc_irq(int dev_id)
 * Decription: Standard rtc handler
 * input: dev_id - currently unused
 * output: none
 * Side effects: Writes to the RTC and updates time counters
 */
void do_rtc_irq(int dev_id) {

    // Reset rtc_flag
    uint8_t task;
    for (task = 0; task < NUM_TASKS; task++) {
        tasks[task]->rtc_counter-= rtc_freq;
    }

    if(!tasks[INIT]->rtc_counter){
        update_time(!num_open && rtc_freq != (MAX_RTC_FREQ >> BASE_RTC_LOG));
    }
    // read port C to acknowledge the interrupt
    outb(CHOOSE_RTC_C, RTC_PORT);
    inb(RTC_PORT + 1);
}
