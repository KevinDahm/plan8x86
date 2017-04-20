/* tuxctl-ioctl.c
 *
 * Driver (skeleton) for the mp2 tuxcontrollers for ECE391 at UIUC.
 *
 * Mark Murphy 2006
 * Andrew Ofisher 2007
 * Steve Lumetta 12-13 Sep 2009
 * Puskar Naha 2013
 */

#include <asm/current.h>
#include <asm/uaccess.h>

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/file.h>
#include <linux/miscdevice.h>
#include <linux/kdev_t.h>
#include <linux/tty.h>
#include <linux/spinlock.h>

#include "tuxctl-ld.h"
#include "tuxctl-ioctl.h"
#include "mtcp.h"

#define debug(str, ...)                                         \
    printk(KERN_DEBUG "%s: " str, __FUNCTION__, ## __VA_ARGS__)

static int tux_set_led(struct tty_struct* tty, unsigned long arg, int force_refresh);

static unsigned char button_state;

static int led;

static int waiting_for_ack = 0;

static void init(struct tty_struct* tty) {
    unsigned char buf[1];

    button_state = 0;

    buf[0] = MTCP_BIOC_ON;
    tuxctl_ldisc_put(tty, buf, 1);

    buf[0] = MTCP_LED_USR;
    tuxctl_ldisc_put(tty, buf, 1);

    waiting_for_ack = 1;
}

/************************ Protocol Implementation *************************/

/* tuxctl_handle_packet()
 * IMPORTANT : Read the header for tuxctl_ldisc_data_callback() in
 * tuxctl-ld.c. It calls this function, so all warnings there apply
 * here as well.
 */
void tuxctl_handle_packet (struct tty_struct* tty, unsigned char* packet)
{
    unsigned char b0 = packet[0];
    unsigned char b1 = packet[1];
    unsigned char b2 = packet[2];
    switch (b0) {

    case MTCP_RESET: {
        init(tty);
        tux_set_led(tty, led, 1);
    } break;

    case MTCP_BIOC_EVENT: {
        b1 = b1 & 0xF;
        b2 = b2 & 0xF;
        // Swap the second and third bit since the format recieved is different than the one we return.
        b2 = (b2 & 0x8) | ((b2 & 0x2) << 1) | ((b2 & 0x4) >> 1) | (b2 & 0x1);
        button_state = ~(b1 | (b2 << 4));
    } break;

    case MTCP_ACK: {
        waiting_for_ack = 0;
    } break;

    }
}

static unsigned char display_bytes[] = {
    0xE7,
    0x06,
    0xCB,
    0x8F,
    0x2E,
    0xAD,
    0xED,
    0x86,
    0xEF,
    0xAF,
    0xEE,
    0x6D,
    0xE1,
    0x4F,
    0xE9,
    0xE8
};

/*
 * tux_set_led
 *    DESCRIPTION: Writes the led data from arg out to the tux controller.
 *    INPUTS: tty - The tux controller
 *            arg - The data to write to the LED recieved from ioctl
 *            force_refresh - In order to not spam the device we only actually update the display
 *                            if arg is different from the last time this function was called
 *                            force_refresh by passes this optimization, for use after a refresh
 *    OUTPUTS: none
 *    SIDE_EFFECTS: Updates the display on the tux controller.
 */
static int tux_set_led(struct tty_struct* tty, unsigned long arg, int force_refresh) {
    int i;
    char on = (arg >> 16) & 0xF;
    char decimals = (arg >> 24) & 0xF;
    unsigned char buf[6];

    // If the display is already showing arg, just return. No need to bother the device.
    if (arg == led && !force_refresh) {
        return 0;
    }
    led = arg;

    for (i = 0; i < 4; i++) {
        /* If the i'th byte of on is set then display something to the screen.
           Otherwise write clear it (write 0).
           display_bytes contains an array of binary values that light the 7-segment
           display correctly for each number. To get the number required shift arg to
           the right by i*4 and mask it against 0xF to get the correct 4 bits.
           Then, to set the decimal point check if the decimal is set by shifting
           decimals to the right by i and masking against 0x1. Shift it back by 4
           to get either 0x10 or 0x00 which is the location of the decimal bit. See
           mtcp.h.
        */
        buf[i+2] = (on & (0x1 << i)) ?
            display_bytes[(arg >> (i << 0x2)) & 0xF] | (((decimals >> i) & 0x1) << 0x4) :
            0x0;
    }

    buf[0] = MTCP_LED_SET;
    buf[1] = 0xF;
    tuxctl_ldisc_put(tty, buf, 6);

    waiting_for_ack = 1;
    return 0;
}

/******** IMPORTANT NOTE: READ THIS BEFORE IMPLEMENTING THE IOCTLS ************
 *                                                                            *
 * The ioctls should not spend any time waiting for responses to the commands *
 * they send to the controller. The data is sent over the serial line at      *
 * 9600 BAUD. At this rate, a byte takes approximately 1 millisecond to       *
 * transmit; this means that there will be about 9 milliseconds between       *
 * the time you request that the low-level serial driver send the             *
 * 6-byte SET_LEDS packet and the time the 3-byte ACK packet finishes         *
 * arriving. This is far too long a time for a system call to take. The       *
 * ioctls should return immediately with success if their parameters are      *
 * valid.                                                                     *
 *                                                                            *
 ******************************************************************************/
int
tuxctl_ioctl (struct tty_struct* tty, struct file* file,
              unsigned cmd, unsigned long arg)
{
    if (!waiting_for_ack) {
        switch (cmd) {
        case TUX_INIT: {
            init(tty);
            return 0;
        } break;

        case TUX_BUTTONS: {
            if (arg) {
                *((char *)arg) = button_state;
            } else {
                return -EINVAL;
            }

        } break;

        case TUX_SET_LED: {
            return tux_set_led(tty, arg, 0);
        } break;

        default:
            return -EINVAL;
        }
    } else {
        return -EINVAL;
    }

    return 0;
}
