/*                                    tab:8
 *
 * modex.c - VGA mode X graphics routines
 *
 * "Copyright (c) 2004-2009 by Steven S. Lumetta."
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * IN NO EVENT SHALL THE AUTHOR OR THE UNIVERSITY OF ILLINOIS BE LIABLE TO
 * ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
 * DAMAGES ARISING OUT  OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
 * EVEN IF THE AUTHOR AND/OR THE UNIVERSITY OF ILLINOIS HAS BEEN ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE AUTHOR AND THE UNIVERSITY OF ILLINOIS SPECIFICALLY DISCLAIM ANY
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND NEITHER THE AUTHOR NOR
 * THE UNIVERSITY OF ILLINOIS HAS ANY OBLIGATION TO PROVIDE MAINTENANCE,
 * SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS."
 *
 * Author:        Steve Lumetta
 * Version:        3
 * Creation Date:   Fri Sep 10 09:59:17 2004
 * Filename:        modex.c
 * History:
 *    SL    1    Fri Sep 10 09:59:17 2004
 *        First written.
 *    SL    2    Sat Sep 12 16:41:45 2009
 *        Integrated original release back into main code base.
 *    SL    3    Sat Sep 12 17:58:20 2009
 *              Added display re-enable to VGA blank routine and comments
 *              on other VirtualPC->QEMU migration changes.
 */
#include "blocks.h"
#include "modex.h"
#include "text.h"
#include "ece391support.h"
#include "ece391syscall.h"

/*
 * Calculate the image build buffer parameters.  SCROLL_SIZE is the space
 * needed for one plane of an image.  SCREEN_SIZE is the space needed for
 * all four planes.  The extra +1 supports logical view x coordinates that
 * are not multiples of four.  In these cases, some plane addresses are
 * shifted by 1 byte forward.  The planes are stored in the build buffer
 * in reverse order to allow those planes that shift forward to do so
 * without running into planes that aren't shifted.  For example, when
 * the leftmost x pixel in the logical view is 3 mod 4, planes 2, 1, and 0
 * are shifted forward, while plane 3 is not, so there is one unused byte
 * between the image of plane 3 and that of plane 2.  BUILD_BUF_SIZE is
 * the size of the space allocated for building images.  We add 20000 bytes
 * to reduce the number of memory copies required during scrolling.
 * Strictly speaking (try it), no extra space is necessary, but the minimum
 * means an extra 64kB memory copy with every scroll pixel.  Finally,
 * BUILD_BASE_INIT places initial (or transferred) logical view in the
 * middle of the available buffer area.
 */
#define SCROLL_SIZE     (SCROLL_X_WIDTH * SCROLL_Y_DIM)
#define SCREEN_SIZE    (SCROLL_SIZE * 4 + 1)
#define BUILD_BUF_SIZE  (SCREEN_SIZE + 20000)
#define BUILD_BASE_INIT ((BUILD_BUF_SIZE - SCREEN_SIZE) / 2)

/* Mode X and general VGA parameters */
#define VID_MEM_SIZE       131072
#define MODE_X_MEM_SIZE     65536
#define NUM_SEQUENCER_REGS      5
#define NUM_CRTC_REGS          25
#define NUM_GRAPHICS_REGS       9
#define NUM_ATTR_REGS          22

/* VGA register settings for mode X */
static unsigned short mode_X_seq[NUM_SEQUENCER_REGS] = {
    0x0100, 0x2101, 0x0F02, 0x0003, 0x0604
};
static unsigned short mode_X_CRTC[NUM_CRTC_REGS] = {
    0x5F00, 0x4F01, 0x5002, 0x8203, 0x5404, 0x8005, 0xBF06, 0x1F07,
    0x0008, 0x0109, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F,
    0x9C10, 0x8E11, 0x8F12, 0x2813, 0x0014, 0x9615, 0xB916, 0xE317,
    0x6B18
};

static unsigned char mode_X_attr[NUM_ATTR_REGS * 2] = {
    0x00, 0x00, 0x01, 0x01, 0x02, 0x02, 0x03, 0x03,
    0x04, 0x04, 0x05, 0x05, 0x06, 0x06, 0x07, 0x07,
    0x08, 0x08, 0x09, 0x09, 0x0A, 0x0A, 0x0B, 0x0B,
    0x0C, 0x0C, 0x0D, 0x0D, 0x0E, 0x0E, 0x0F, 0x0F,
    0x10, 0x61, 0x11, 0x00, 0x12, 0x0F, 0x13, 0x00,
    0x14, 0x00, 0x15, 0x00
};
static unsigned short mode_X_graphics[NUM_GRAPHICS_REGS] = {
    0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x4005, 0x0506, 0x0F07,
    0xFF08
};

/* VGA register settings for text mode 3 (color text) */
static unsigned short text_seq[NUM_SEQUENCER_REGS] = {
    0x0100, 0x2001, 0x0302, 0x0003, 0x0204
};
static unsigned short text_CRTC[NUM_CRTC_REGS] = {
    0x5F00, 0x4F01, 0x5002, 0x8203, 0x5504, 0x8105, 0xBF06, 0x1F07,
    0x0008, 0x4F09, 0x0D0A, 0x0E0B, 0x000C, 0x000D, 0x000E, 0x000F,
    0x9C10, 0x8E11, 0x8F12, 0x2813, 0x1F14, 0x9615, 0xB916, 0xA317,
    0xFF18
};
static unsigned char text_attr[NUM_ATTR_REGS * 2] = {
    0x00, 0x00, 0x01, 0x01, 0x02, 0x02, 0x03, 0x03,
    0x04, 0x04, 0x05, 0x05, 0x06, 0x06, 0x07, 0x07,
    0x08, 0x08, 0x09, 0x09, 0x0A, 0x0A, 0x0B, 0x0B,
    0x0C, 0x0C, 0x0D, 0x0D, 0x0E, 0x0E, 0x0F, 0x0F,
    0x10, 0x0C, 0x11, 0x00, 0x12, 0x0F, 0x13, 0x08,
    0x14, 0x00, 0x15, 0x00
};
static unsigned short text_graphics[NUM_GRAPHICS_REGS] = {
    0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x1005, 0x0E06, 0x0007,
    0xFF08
};

/* local functions--see function headers for details */
static int open_memory_and_ports ();
static void VGA_blank (int blank_bit);
static void set_seq_regs_and_reset (unsigned short table[NUM_SEQUENCER_REGS],
                                    unsigned char val);
static void set_CRTC_registers (unsigned short table[NUM_CRTC_REGS]);
static void set_attr_registers (unsigned char table[NUM_ATTR_REGS * 2]);
static void set_graphics_registers (unsigned short table[NUM_GRAPHICS_REGS]);
static void fill_palette ();
static void write_font_data ();
static void set_text_mode_3 (int clear_scr);
static void copy_image (unsigned char* img, unsigned short scr_addr);
static void copy_image_statusbar (unsigned char* img);
static void undraw_masked_blocks();
static void change_pallette(short pallete_index, unsigned char r, unsigned char g, unsigned char b);

/*
 * Images are built in this buffer, then copied to the video memory.
 * Copying to video memory with REP MOVSB is vastly faster than anything
 * else with emulation, probably because it is a single instruction
 * and translates to a native loop.  It's also a pretty good technique
 * in normal machines (albeit not as elegant as some others for reducing
 * the number of video memory writes; unfortunately, these techniques
 * are slower in emulation...).
 *
 * The size allows the four plane images to move within an area of
 * about twice the size necessary (to reduce the need to deal with
 * the boundary conditions by moving the data within the buffer).
 *
 * Plane 3 is first, followed by 2, 1, and 0.  The reverse ordering
 * is used because the logical address of 0 increases first; if plane
 * 0 were first, we would need a buffer byte to keep it from colliding
 * with plane 1 when plane 0 was offset by 1 from plane 1, i.e., when
 * displaying a one-pixel left shift.
 *
 * The memory fence (included when NDEBUG is not defined) allocates
 * the build buffer with extra space on each side.  The extra space
 * is filled with magic numbers (something unlikely to be written in
 * error), and the fence areas are checked for those magic values at
 * the end of the program to detect array access bugs (writes past
 * the ends of the build buffer).
 */
#if !defined(NDEBUG)
#define MEM_FENCE_WIDTH 256
#else
#define MEM_FENCE_WIDTH 0
#endif
#define MEM_FENCE_MAGIC 0xF3
static unsigned char build[BUILD_BUF_SIZE + 2 * MEM_FENCE_WIDTH];
static int img3_off;            /* offset of upper left pixel   */
static unsigned char* img3;        /* pointer to upper left pixel  */
static int show_x, show_y;          /* logical view coordinates     */

/*
 * The build buffer for storing the statusbar image data
 * This buffer follows the same format as build with the exception
 * that the first plane is always at index 0 in the buffer since the
 * statusbar is setup to never scroll.
 */
#define STATUSBAR_BUF_SIZE (STATUSBAR_Y_DIM * IMAGE_X_DIM)
static unsigned char sb_build[STATUSBAR_BUF_SIZE];

/* displayed video memory variables */
static unsigned char* mem_image;    /* pointer to start of video memory */
static unsigned short target_img;   /* offset of displayed screen image */

#define STATUSBAR_COLOR 0x24
#define TEXT_COLOR 0x23

/*
 * functions provided by the caller to set_mode_X() and used to obtain
 * graphic images of lines (pixels) to be mapped into the build buffer
 * planes for display in mode X
 */
static void (*horiz_line_fn) (int, int, unsigned char[SCROLL_X_DIM]);
static void (*vert_line_fn) (int, int, unsigned char[SCROLL_Y_DIM]);


/*
 * macro used to target a specific video plane or planes when writing
 * to video memory in mode X; bits 8-11 in the mask_hi_bits enable writes
 * to planes 0-3, respectively
 */
#define SET_WRITE_MASK(mask_hi_bits)                                  \
    do {                                                              \
        asm volatile ("                                                     \
    movw $0x03C4,%%dx        /* set write mask                    */;\
    movb $0x02,%b0                                                 ;\
    outw %w0,(%%dx)                                                 \
    " : : "a" ((mask_hi_bits)) : "edx", "memory");                    \
    } while (0)

/* macro used to write a byte to a port */
#define OUTB(port,val)                            \
    do {                                          \
        asm volatile ("                                                     \
        outb %b1,(%w0)                                                  \
    " : /* no outputs */                          \
                      : "d" ((port)), "a" ((val)) \
                      : "memory", "cc");          \
    } while (0)

/* macro used to write two bytes to two consecutive ports */
#define OUTW(port,val)                            \
    do {                                          \
        asm volatile ("                                                     \
        outw %w1,(%w0)                                                  \
    " : /* no outputs */                          \
                      : "d" ((port)), "a" ((val)) \
                      : "memory", "cc");          \
    } while (0)

/*
 * macro used to write an array of two-byte values to two consecutive ports
 */
#define REP_OUTSW(port,source,count)                                    \
    do {                                                                \
        asm volatile ("                                                     \
     1: movw 0(%1),%%ax                                                ;\
    outw %%ax,(%w2)                                                ;\
    addl $2,%1                                                     ;\
    decl %0                                                        ;\
    jne 1b                                                          \
    " : /* no outputs */                                                \
                      : "c" ((count)), "S" ((source)), "d" ((port))     \
                      : "eax", "memory", "cc");                         \
    } while (0)

/*
 * macro used to write an array of one-byte values to two consecutive ports
 */
#define REP_OUTSB(port,source,count)                                    \
    do {                                                                \
        asm volatile ("                                                     \
     1: movb 0(%1),%%al                                                ;\
    outb %%al,(%w2)                                                ;\
    incl %1                                                        ;\
    decl %0                                                        ;\
    jne 1b                                                          \
    " : /* no outputs */                                                \
                      : "c" ((count)), "S" ((source)), "d" ((port))     \
                      : "eax", "memory", "cc");                         \
    } while (0)


/*
 * sbIndexToBuildIndex
 *   DESCRIPTION: Converts an index for a STATUSBAR_X_DIM * STATUSBAR_Y_DIM array into
 *                an index for sb_build. Will be inlined, very simple calculation.
 *   INPUTS: sbIndex - The index to be converted
 *   OUTPUTS: none
 *   RETURN VALUE: An index into sb_build at the correct location
 *   SIDE EFFECTS: none
 */
int sbIndexToBuildIndex(int sbIndex) {
    return (sbIndex >> 2) + (sbIndex & 3) * (STATUSBAR_BUF_SIZE >> 2);
}

/*
 * drawBuffer_statusbar
 *   DESCRIPTION: Writes pixels in buf to sb_build to be blitted to the statusbar on the next frame
 *   INPUTS: buf - an array of pixels
 *           x   - The x location of the top left corner in statusbar coordinates
 *           y   - The y location of the top left corner in statusbar coordinates
 *           w   - The width of buf
 *           h   - The height of buf
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success
 *   SIDE EFFECTS: Changes sb_build to contain the contents of buffer at x, y
 */
int drawBuffer_statusbar(const unsigned char *buf, int x, int y, int w, int h) {
    int r, c;
    for (r = 0; r < h; r++) {
        for (c = 0; c < w; c++) {
            int buildIndex = sbIndexToBuildIndex((r + y) * STATUSBAR_X_DIM + (c + x));
            sb_build[buildIndex] = buf[r * w + c];
        }
    }
    return 0;
}

/*
 * drawText_statusbar
 *   DESCRIPTION: Draws text to sb_build to be blitted to the statusbar on next frame
 *   INPUTS: str - The string of text to be blitted
 *           length - The length of str
 *           fg  - Foreground color
 *           bg  - Background color
 *           x   - The x location of the top left corner in statusbar coordinates
 *           y   - The y location of the top left corner in statusbar coordinates
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success
 *   SIDE EFFECTS: Changes sb_build to contain the text at x, y
 */
int drawText_statusbar(const char *str, const int length, unsigned char fg, unsigned char bg, int x, int y) {
    unsigned char buf[FONT_HEIGHT*FONT_WIDTH*length];
    create_bitmap_from_string(buf, (unsigned char *)str, length, fg, bg);
    drawBuffer_statusbar(buf, x, y, FONT_WIDTH*length, FONT_HEIGHT);
    return 0;
}

/*
 * fillColor_statusbar
 *   DESCRIPTION: Fills sb_build with a solid color
 *   INPUTS: color - The color to fill sb_build with
 *           x   - The x location of the top left corner in statusbar coordinates
 *           y   - The y location of the top left corner in statusbar coordinates
 *           w   - The width of the fill region
 *           h   - The height of the fill region
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success
 *   SIDE EFFECTS: Changes sb_build to contain the color color in the rectangle (x, y, w, h)
 */
int fillColor_statusbar(unsigned char color, int x, int y, int w, int h) {
    int i, j;
    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            int buildIndex = sbIndexToBuildIndex((i + y) * STATUSBAR_X_DIM + (j + x));
            sb_build[buildIndex] = color;
        }
    }
    return 0;
}



/*
 * draw_statusbar
 *   DESCRIPTION: Fills sb_build with a formated string with the current level
 *                number of fruit and time elapsed. Uses STATUSBAR_COLOR for the background
 *                and TEXT_COLOR for the foreground
 *   INPUTS: level       - The current level
 *           secs        - The elapsed time. To be formated in minute:seconds format
 *           fruit_count - The number of fruit left to get
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success
 *   SIDE EFFECTS: Completely overwrites everything in sb_build
 */
int draw_statusbar(unsigned int level, unsigned int secs, unsigned int fruit_count) {
    fillColor_statusbar(STATUSBAR_COLOR, 0, 0, STATUSBAR_X_DIM, STATUSBAR_Y_DIM);

    /* if (fruit_count == 1) { */
    /*     // The formated string will be either 27 or 28 characters long, depending on the level */
    /*     // Allocate space for the longer one so sprintf does not crash. */
    /*     char sb_line[28]; */
    /*     sprintf(sb_line, "Level %d    1 Fruit    %02d:%02d", level, secs / 60, secs % 60); */

    /*     // If we are on level 10 the string is length 28, otherwise sprintf didn't fill it all the way and it's length 27 */
    /*     const int length = (level < 10 ? 27 : 28); */

    /*     int x = (STATUSBAR_X_DIM / 2) - (FONT_WIDTH * length / 2); */

    /*     drawText_statusbar(sb_line, length, TEXT_COLOR, STATUSBAR_COLOR, x, 1); */
    /* } else { */
    /*     // The formated string will be either 28 or 29 characters long, depending on the level */
    /*     // Allocate space for the longer one so sprintf does not crash. */
    /*     char sb_line[29]; */
    /*     sprintf(sb_line, "Level %d    %d Fruits    %02d:%02d", level, fruit_count, secs / 60, secs % 60); */

    /*     // If we are on level 10 the string is length 29, otherwise sprintf didn't fill it all the way and it's length 28 */
    /*     const int length = (level < 10 ? 28 : 29); */
    /*     int x = (STATUSBAR_X_DIM / 2) - (FONT_WIDTH * length / 2); */

    /*     drawText_statusbar(sb_line, length, TEXT_COLOR, STATUSBAR_COLOR, x, 1); */
    /* } */
    return 0;
}

/*
 * set_mode_X
 *   DESCRIPTION: Puts the VGA into mode X.
 *   INPUTS: horiz_fill_fn -- this function is used as a callback (by
 *                     draw_horiz_line) to obtain a graphical
 *                     image of a particular logical line for
 *                     drawing to the build buffer
 *           vert_fill_fn -- this function is used as a callback (by
 *                    draw_vert_line) to obtain a graphical
 *                    image of a particular logical line for
 *                    drawing to the build buffer
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: initializes the logical view window; maps video memory
 *                 and obtains permission for VGA ports; clears video memory
 */
int
set_mode_X (void (*horiz_fill_fn) (int, int, unsigned char[SCROLL_X_DIM]),
            void (*vert_fill_fn) (int, int, unsigned char[SCROLL_Y_DIM]))
{
    int i; /* loop index for filling memory fence with magic numbers */

    fillColor_statusbar(STATUSBAR_COLOR, 0, 0, STATUSBAR_X_DIM, STATUSBAR_Y_DIM);

    /*
     * Record callback functions for obtaining horizontal and vertical
     * line images.
     */
    if (horiz_fill_fn == NULL || vert_fill_fn == NULL)
        return -1;
    horiz_line_fn = horiz_fill_fn;
    vert_line_fn = vert_fill_fn;

    /* Initialize the logical view window to position (0,0). */
    show_x = show_y = 0;
    img3_off = BUILD_BASE_INIT;
    img3 = build + img3_off + MEM_FENCE_WIDTH;

    /* Set up the memory fence on the build buffer. */
    for (i = 0; i < MEM_FENCE_WIDTH; i++) {
        build[i] = MEM_FENCE_MAGIC;
        build[BUILD_BUF_SIZE + MEM_FENCE_WIDTH + i] = MEM_FENCE_MAGIC;
    }

    /* One display page goes at the start of video memory. */
    /* Offset target_img by the statusbar_plane_size so that data from build
       is not overwritting the statusbar */
    target_img = STATUSBAR_PLANE_SIZE;

    /* Map video memory and obtain permission for VGA port access. */
    open_memory_and_ports ();

    /*
     * The code below was produced by recording a call to set mode 0013h
     * with display memory clearing and a windowed frame buffer, then
     * modifying the code to set mode X instead.  The code was then
     * generalized into functions...
     *
     * modifications from mode 13h to mode X include...
     *   Sequencer Memory Mode Register: 0x0E to 0x06 (0x3C4/0x04)
     *   Underline Location Register   : 0x40 to 0x00 (0x3D4/0x14)
     *   CRTC Mode Control Register    : 0xA3 to 0xE3 (0x3D4/0x17)
     */

    VGA_blank (1);                               /* blank the screen      */
    set_seq_regs_and_reset (mode_X_seq, 0x63);   /* sequencer registers   */
    set_CRTC_registers (mode_X_CRTC);            /* CRT control registers */
    set_attr_registers (mode_X_attr);            /* attribute registers   */
    set_graphics_registers (mode_X_graphics);    /* graphics registers    */
    fill_palette ();                             /* palette colors        */
    clear_screens ();                            /* zero video memory     */
    VGA_blank (0);                               /* unblank the screen    */

    /* Return success. */
    return 0;
}



/*
 * clear_mode_X
 *   DESCRIPTION: Puts the VGA into text mode 3 (color text).
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: restores font data to video memory; clears screens;
 *                 unmaps video memory; checks memory fence integrity
 */
void
clear_mode_X ()
{
    int i;   /* loop index for checking memory fence */

    /* Put VGA into text mode, restore font data, and clear screens. */
    set_text_mode_3 (1);

    /* Unmap video memory. */
    /* (void)munmap (mem_image, VID_MEM_SIZE); */

    /* Check validity of build buffer memory fence.  Report breakage. */
    for (i = 0; i < MEM_FENCE_WIDTH; i++) {
        if (build[i] != MEM_FENCE_MAGIC) {
            /* puts ("lower build fence was broken"); */
            break;
        }
    }
    for (i = 0; i < MEM_FENCE_WIDTH; i++) {
        if (build[BUILD_BUF_SIZE + MEM_FENCE_WIDTH + i] != MEM_FENCE_MAGIC) {
            /* puts ("upper build fence was broken"); */
            break;
        }
    }
}


/*
 * set_view_window
 *   DESCRIPTION: Set the logical view window, moving its location within
 *                the build buffer if necessary to keep all on-screen data
 *                in the build buffer.  If the location within the build
 *                buffer moves, this function copies all data from the old
 *                window that are within the new screen to the appropriate
 *                new location, so only data not previously on the screen
 *                must be drawn before calling show_screen.
 *   INPUTS: (scr_x,scr_y) -- new upper left pixel of logical view window
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: may shift position of logical view window within build
 *                 buffer
 */
void
set_view_window (int scr_x, int scr_y)
{
    int old_x, old_y;     /* old position of logical view window           */
    int start_x, start_y; /* starting position for copying from old to new */
    int end_x, end_y;     /* ending position for copying from old to new   */
    int start_off;        /* offset of copy start relative to old build    */
    /*    buffer start position                      */
    int length;           /* amount of data to be copied                   */
    int i;              /* copy loop index                               */
    unsigned char* start_addr;  /* starting memory address of copy     */
    unsigned char* target_addr; /* destination memory address for copy */

    /* Record the old position. */
    old_x = show_x;
    old_y = show_y;

    /* Keep track of the new view window. */
    show_x = scr_x;
    show_y = scr_y;

    /*
     * If the new view window fits within the boundaries of the build
     * buffer, we need move nothing around.
     */
    if (img3_off + (scr_x >> 2) + scr_y * SCROLL_X_WIDTH >= 0 &&
        img3_off + 3 * SCROLL_SIZE +
        ((scr_x + SCROLL_X_DIM - 1) >> 2) +
        (scr_y + SCROLL_Y_DIM - 1) * SCROLL_X_WIDTH < BUILD_BUF_SIZE)
        return;

    /*
     * If the new screen does not overlap at all with the old screen, none
     * of the old data need to be saved, and we can simply reposition the
     * valid window of the build buffer in the middle of that buffer.
     */
    if (scr_x <= old_x - SCROLL_X_DIM || scr_x >= old_x + SCROLL_X_DIM ||
        scr_y <= old_y - SCROLL_Y_DIM || scr_y >= old_y + SCROLL_Y_DIM) {
        img3_off = BUILD_BASE_INIT - (scr_x >> 2) - scr_y * SCROLL_X_WIDTH;
        img3 = build + img3_off + MEM_FENCE_WIDTH;
        return;
    }

    /*
     * Any still-visible portion of the old screen should be retained.
     * Rather than clipping exactly, we copy all contiguous data between
     * a clipped starting point to a clipped ending point (which may
     * include non-visible data).
     *
     * The starting point is the maximum (x,y) coordinates between the
     * new and old screens.  The ending point is the minimum (x,y)
     * coordinates between the old and new screens (offset by the screen
     * size).
     */
    if (scr_x > old_x) {
        start_x = scr_x;
        end_x = old_x;
    } else {
        start_x = old_x;
        end_x = scr_x;
    }
    end_x += SCROLL_X_DIM - 1;
    if (scr_y > old_y) {
        start_y = scr_y;
        end_y = old_y;
    } else {
        start_y = old_y;
        end_y = scr_y;
    }
    end_y += SCROLL_Y_DIM - 1;

    /*
     * We now calculate the starting and ending addresses for the copy
     * as well as the new offsets for use with the build buffer.  The
     * length to be copied is basically the ending offset minus the starting
     * offset plus one (plus the three screens in between planes 3 and 0).
     */
    start_off = (start_x >> 2) + start_y * SCROLL_X_WIDTH;
    start_addr = img3 + start_off;
    length = (end_x >> 2) + end_y * SCROLL_X_WIDTH + 1 - start_off +
        3 * SCROLL_SIZE;
    img3_off = BUILD_BASE_INIT - (show_x >> 2) - show_y * SCROLL_X_WIDTH;
    img3 = build + img3_off + MEM_FENCE_WIDTH;
    target_addr = img3 + start_off;

    /*
     * Copy the relevant portion of the screen from the old location to the
     * new one.  The areas may overlap, so copy direction is important.
     * (You should be able to explain why!)
     */
    if (start_addr < target_addr)
        for (i = length; i-- > 0; )
            target_addr[i] = start_addr[i];
    else
        for (i = 0; i < length; i++)
            target_addr[i] = start_addr[i];
}


/*
 * show_statusbar
 *   DESCRIPTION: Show the statusbar on the video display.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: copies sb_build buffer to video memory;
 */
void show_statusbar() {
    int i;

    for (i = 0; i < 4; i++) {
        SET_WRITE_MASK(1 << (i + 8));
        copy_image_statusbar(sb_build + i * (STATUSBAR_BUF_SIZE >> 2));
    }
}

/*
 * show_screen
 *   DESCRIPTION: Show the logical view window on the video display.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: copies from the build buffer to video memory;
 *                 shifts the VGA display source to point to the new image
 */
void
show_screen ()
{
    unsigned char* addr;  /* source address for copy             */
    int p_off;            /* plane offset of first display plane */
    int i;          /* loop index over video planes        */

    /*
     * Calculate offset of build buffer plane to be mapped into plane 0
     * of display.
     */
    p_off = (3 - (show_x & 3));

    /* Switch to the other target screen in video memory. */
    target_img ^= 0x4000; // Nice magic number ty

    /* Calculate the source address. */
    addr = img3 + (show_x >> 2) + show_y * SCROLL_X_WIDTH;

    /* Draw to each plane in the video memory. */
    for (i = 0; i < 4; i++) {
        SET_WRITE_MASK (1 << (i + 8));
        copy_image (addr + ((p_off - i + 4) & 3) * SCROLL_SIZE + (p_off < i),
                    target_img);
    }

    /*
     * Change the VGA registers to point the top left of the screen
     * to the video memory that we just filled.
     */
    OUTW (0x03D4, (target_img & 0xFF00) | 0x0C);
    OUTW (0x03D4, ((target_img & 0x00FF) << 8) | 0x0D);

    // Must be done after each frame is drawn in order for masking to work
    undraw_masked_blocks();
}


/*
 * clear_screens
 *   DESCRIPTION: Fills the video memory with zeroes.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: fills all 256kB of VGA video memory with zeroes
 */
void
clear_screens ()
{
    /* Write to all four planes at once. */
    SET_WRITE_MASK (0x0F00);

    /* Set 64kB to zero (times four planes = 256kB). */
    memset (mem_image, 0, MODE_X_MEM_SIZE);
}

// Define a special color that draw_full_block uses to detect transparency.
#define TRANSPARENT 0xFF


/*
 * draw_text_above_player
 *    DESCRIPTION: Draws text above the player's head.
 *    INPUTS: str - The text to draw
 *            length - The length of the string to draw
 *            (play_x, play_y) - The player coordinates to draw above
 *    OUTPUTS: none
 *    RETURN VALUE: 0 on success
 *    SIDE EFFECTS: calls draw_full_block. So it will write into the build buffer and generate some undraw data.
 */
int draw_text_above_player(const char *str, const int length, int play_x, int play_y) {
    const int buf_size = length * FONT_HEIGHT * FONT_WIDTH;
    unsigned char buf[buf_size];
    create_bitmap_from_string(buf, (const unsigned char *)str, length, TRANSPARENT, 0x0);
    unsigned char mask[buf_size];
    int i;
    for (i = 0; i < buf_size; i++) {
        mask[i] = (unsigned char)(buf[i] != 0);
    }
    int y = play_y - FONT_HEIGHT < show_y ? show_y : play_y - FONT_HEIGHT;
    draw_full_block(play_x + (BLOCK_X_DIM / 2) - length * FONT_WIDTH / 2, y,
                    FONT_WIDTH * length, FONT_HEIGHT, buf, mask);
    return 0;
}

// A record of a block that will need to be redrawn
typedef struct undraw_block_node undraw_block_node;
struct undraw_block_node {
    unsigned char *data;
    int x, y;
    int w, h;
    undraw_block_node *next;
};
// A linked list of blocks that need to be undrawn after the screen is shown.
static undraw_block_node *undraw_block_list;

/*
 * undraw_masked_blocks
 *   DESCRIPTION: traverses the undraw_block_list and writes each node's data into the build buffer.
 *                NOTE: Should only be called at the end of show_screen
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: draws into the build buffer and clears undraw_block_list
 */
void undraw_masked_blocks() {
    if (!undraw_block_list) {
        return;
    }

    int y, x;
    undraw_block_node *curr_node = undraw_block_list;
    do {
        // Put all the data in curr_node->data back into the build buffer.
        // These are the pixels present before the masked block was drawn, see draw_full_block
        for (y = 0; y < curr_node->h; y++) {
            for (x = 0; x < curr_node->w; x++) {
                *(img3 + ((x + curr_node->x) >> 2) + (y + curr_node->y) * SCROLL_X_WIDTH +
                  (3 - ((x + curr_node->x) & 3)) * SCROLL_SIZE) = curr_node->data[x + y * curr_node->w];
            }
        }
        /* undraw_block_node *old = curr_node; */
        curr_node = curr_node->next;
        /* free(old->data); */
        /* free(old); */
    } while (curr_node);
    undraw_block_list = NULL;
}

#define WALL_TEXT_COLOR 0x45
#define WALL_FOG_COLOR 0x46

// 8 colors for the player center color and the walls for each level.
static unsigned char color_cycle[8][3] = {
    {0x00, 0x00, 0x3F}, {0x3F, 0x00, 0x00},
    {0x3B, 0x00, 0x3F}, {0x01, 0x28, 0x00},
    {0x3E, 0x3F, 0x00}, {0x3F, 0x21, 0x00},
    {0x00, 0x3E, 0x3F}, {0x2D, 0x00, 0x3F},
};
// The same 8 colors but 50% whiter. This gives the effect of transparency.
static unsigned char color_cycle_transparent[8][3] = {
    {0x20, 0x20, 0x3F}, {0x3F, 0x20, 0x20},
    {0x3D, 0x1F, 0x3F}, {0x15, 0x28, 0x14},
    {0x3F, 0x3F, 0x20}, {0x3F, 0x30, 0x20},
    {0x20, 0x3F, 0x3F}, {0x36, 0x1F, 0x3F},
};

/*
 * update_player_glow_color
 *    DESCRIPTION: Changes the color of the center of the player
 *    INPUTS: none
 *    OUTPUTS: none
 *    RETURN VALUE: none
 *    SIDE EFFECTS: changes the pallette.
 */
void update_player_glow_color() {
    static int i = 0;
    change_pallette(PLAYER_CENTER_COLOR, color_cycle[i][0], color_cycle[i][1], color_cycle[i][2]);
    // Cycle the color once it gets past 7
    i = (i + 1) & 7;
}

/*
 * update_wall_color
 *    DESCRIPTION: Changes the color of the wall, the statusbar, and the transparent text
 *    INPUTS: none
 *    OUTPUTS: none
 *    RETURN VALUE: none
 *    SIDE EFFECTS: changes the pallette.
 */
void update_wall_color() {
    static int i = 0;
    change_pallette(WALL_FILL_COLOR, color_cycle[i][0], color_cycle[i][1], color_cycle[i][2]);
    change_pallette(STATUSBAR_COLOR, color_cycle_transparent[i][0], color_cycle_transparent[i][1], color_cycle_transparent[i][2]);
    change_pallette(WALL_TEXT_COLOR, color_cycle_transparent[i][0], color_cycle_transparent[i][1], color_cycle_transparent[i][2]);
    // Cycle the color once it gets past 7
    i = (i + 1) & 7;
}


/*
 * draw_full_block
 *   DESCRIPTION: Draw a BLOCK_X_DIM x BLOCK_Y_DIM block at absolute
 *                coordinates.  Mask any portion of the block not inside
 *                the logical view window. Also masks any portion of the block
 *                specified by mask if mask is not null
 *   INPUTS: (pos_x,pos_y) -- coordinates of upper left corner of block
 *           (block_w, block_h) -- The size of the block
 *           blk -- image data for block (one byte per pixel, as a C array
 *                  of dimensions [block_h][block_w])
 *           mask -- Mask data as a C array of dimensions [block_h][block_w]
 *                   0 is transparent, non-0 is opaque.
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: draws into the build buffer and if mask is not NULL
 *                 saves previous values of build in the undraw_block_list linked list.
 */
void
draw_full_block (int pos_x, int pos_y, int block_w, int block_h, unsigned char* blk, unsigned char *mask)
{
    int dx, dy;          /* loop indices for x and y traversal of block */
    int x_left, x_right; /* clipping limits in horizontal dimension     */
    int y_top, y_bottom; /* clipping limits in vertical dimension       */

    // If mask is not NULL save the data of the block being overwitten so we can undraw it later
    /* if (mask) { */
    /*     undraw_block_node *b = malloc(sizeof(undraw_block_node)); */
    /*     b->x = pos_x; */
    /*     b->y = pos_y; */
    /*     b->w = block_w; */
    /*     b->h = block_h; */
    /*     b->data = malloc(block_w * block_h); */

    /*     // Prepend b to the list */
    /*     if (undraw_block_list) { */
    /*         b->next = undraw_block_list; */
    /*     } else { */
    /*         b->next = NULL; */
    /*     } */
    /*     undraw_block_list = b; */

    /*     for (dy = 0; dy < block_h; dy++) { */
    /*         for (dx = 0; dx < block_w; dx++) { */
    /*             b->data[dx + dy * block_w] = *(img3 + ((pos_x + dx) >> 2) + (pos_y + dy) * SCROLL_X_WIDTH + */
    /*                                                           (3 - ((pos_x + dx) & 3)) * SCROLL_SIZE); */
    /*         } */
    /*     } */
    /* } */

    /* If block is completely off-screen, we do nothing. */
    if (pos_x + block_w <= show_x || pos_x >= show_x + SCROLL_X_DIM ||
        pos_y + block_h <= show_y || pos_y >= show_y + SCROLL_Y_DIM)
        return;

    /* Clip any pixels falling off the left side of screen. */
    if ((x_left = show_x - pos_x) < 0)
        x_left = 0;
    /* Clip any pixels falling off the right side of screen. */
    if ((x_right = show_x + SCROLL_X_DIM - pos_x) > block_w)
        x_right = block_w;
    /* Skip the first x_left pixels in both screen position and block data. */
    pos_x += x_left;
    blk += x_left;
    if (mask)
        mask += x_left;
    /*
     * Adjust x_right to hold the number of pixels to be drawn, and x_left
     * to hold the amount to skip between rows in the block, which is the
     * sum of the original left clip and (block_w - the original right
     * clip).
     */
    x_right -= x_left;
    x_left = block_w - x_right;

    /* Clip any pixels falling off the top of the screen. */
    if ((y_top = show_y - pos_y) < 0)
        y_top = 0;
    /* Clip any pixels falling off the bottom of the screen. */
    if ((y_bottom = show_y + SCROLL_Y_DIM - pos_y) > block_h)
        y_bottom = block_h;
    /*
     * Skip the first y_left pixel in screen position and the first
     * y_left rows of pixels in the block data.
     */
    pos_y += y_top;
    blk += y_top * block_w;
    if (mask)
        mask += y_top * block_w;
    /* Adjust y_bottom to hold the number of pixel rows to be drawn. */
    y_bottom -= y_top;

    /* Draw the clipped image. */
    for (dy = 0; dy < y_bottom; dy++, pos_y++) {
        for (dx = 0; dx < x_right; dx++, pos_x++, blk++) {
            // If the mask is set or if there is no mask write *blk to build.
            if ((mask && *mask) || !mask) {
                unsigned char c = *blk;

                // If c is the special transparency color replace whatever is in the build buffer currently with
                // a new whiter color. 0x30 - 0x34 are the ground colors and 0x40 - 0x44 are the transparent ground colors
                if (c == TRANSPARENT) {
                    c = *(img3 + (pos_x >> 2) + pos_y * SCROLL_X_WIDTH +
                          (3 - (pos_x & 3)) * SCROLL_SIZE);
                    if (c >= 0x30 && c <= 0x34) {
                        c += 0x10;
                    } else if (c == WALL_FILL_COLOR) {
                        c = WALL_TEXT_COLOR;
                    } else {
                        c = WALL_FOG_COLOR;
                    }
                } else {
                    c = *blk;
                }
                *(img3 + (pos_x >> 2) + pos_y * SCROLL_X_WIDTH +
                  (3 - (pos_x & 3)) * SCROLL_SIZE) = c;
            }
            if (mask)
                mask++;
        }
        pos_x -= x_right;
        blk += x_left;
        if (mask)
            mask += x_left;
    }
}


/*
 * The functions inside the preprocessor block below rely on functions
 * in maze.c to generate graphical images of the maze.  These functions
 * are neither available nor necessary for the text restoration program
 * based on this file, and are omitted to simplify linking that program.
 */
#if !defined(TEXT_RESTORE_PROGRAM)


/*
 * draw_vert_line
 *   DESCRIPTION: Draw a vertical map line into the build buffer.  The
 *                line should be offset from the left side of the logical
 *                view window screen by the given number of pixels.
 *   INPUTS: x -- the 0-based pixel column number of the line to be drawn
 *                within the logical view window (equivalent to the number
 *                of pixels from the leftmost pixel to the line to be
 *                drawn)
 *   OUTPUTS: none
 *   RETURN VALUE: Returns 0 on success.  If x is outside of the valid
 *                 SCROLL range, the function returns -1.
 *   SIDE EFFECTS: draws into the build buffer
 */
int
draw_vert_line (int x)
{
    unsigned char buf[SCROLL_Y_DIM];
    unsigned char* addr;

    int p_off;
    int i;

    if (x < 0 || x >= SCROLL_X_DIM) {
        return -1;
    }

    x += show_x;

    (*vert_line_fn)(x, show_y, buf);

    addr = img3 + (x >> 2) + show_y * SCROLL_X_WIDTH;

    p_off = (3 - (x & 3));

    for (i = 0; i < SCROLL_Y_DIM; i++) {
        addr[p_off * SCROLL_SIZE] = buf[i];
        addr += SCROLL_X_WIDTH;
    }

    return 0;
}


/*
 * draw_horiz_line
 *   DESCRIPTION: Draw a horizontal map line into the build buffer.  The
 *                line should be offset from the top of the logical view
 *                window screen by the given number of pixels.
 *   INPUTS: y -- the 0-based pixel row number of the line to be drawn
 *                within the logical view window (equivalent to the number
 *                of pixels from the top pixel to the line to be drawn)
 *   OUTPUTS: none
 *   RETURN VALUE: Returns 0 on success.  If y is outside of the valid
 *                 SCROLL range, the function returns -1.
 *   SIDE EFFECTS: draws into the build buffer
 */
int
draw_horiz_line (int y)
{
    unsigned char buf[SCROLL_X_DIM]; /* buffer for graphical image of line */
    unsigned char* addr;             /* address of first pixel in build    */
    /*     buffer (without plane offset)  */
    int p_off;                       /* offset of plane of first pixel     */
    int i;                 /* loop index over pixels             */

    /* Check whether requested line falls in the logical view window. */
    if (y < 0 || y >= SCROLL_Y_DIM)
        return -1;

    /* Adjust y to the logical row value. */
    y += show_y;

    /* Get the image of the line. */
    (*horiz_line_fn) (show_x, y, buf);

    /* Calculate starting address in build buffer. */
    addr = img3 + (show_x >> 2) + y * SCROLL_X_WIDTH;

    /* Calculate plane offset of first pixel. */
    p_off = (3 - (show_x & 3));

    /* Copy image data into appropriate planes in build buffer. */
    for (i = 0; i < SCROLL_X_DIM; i++) {
        addr[p_off * SCROLL_SIZE] = buf[i];
        if (--p_off < 0) {
            p_off = 3;
            addr++;
        }
    }

    /* Return success. */
    return 0;
}

#endif /* !defined(TEXT_RESTORE_PROGRAM) */


/*
 * open_memory_and_ports
 *   DESCRIPTION: Map video memory into our address space; obtain permission
 *                to access VGA ports.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: prints an error message to stdout on failure
 */
static int
open_memory_and_ports ()
{

    vidmap_all(&mem_image);
    /* int mem_fd;  /\* file descriptor for physical memory image *\/ */

    /* Obtain permission to access ports 0x03C0 through 0x03DA. */
    if (ioperm (0x03C0, 0x03DA - 0x03C0 + 1, 1) == -1) {
        return -1;
    }

    /* /\* Open file to access physical memory. *\/ */
    /* if ((mem_fd = open ("/dev/mem", O_RDWR)) == -1) { */
    /*     perror ("open /dev/mem"); */
    /*     return -1; */
    /* } */

    /* /\* Map video memory (0xA0000 - 0xBFFFF) into our address space. *\/ */
    /* if ((mem_image = mmap (0, VID_MEM_SIZE, PROT_READ | PROT_WRITE, */
    /*                        MAP_SHARED, mem_fd, 0xA0000)) == MAP_FAILED) { */
    /*     perror ("mmap video memory"); */
    /*     return -1; */
    /* } */

    /* /\* Close /dev/mem file descriptor and return success. *\/ */
    /* (void)close (mem_fd); */
    return 0;
}


/*
 * VGA_blank
 *   DESCRIPTION: Blank or unblank the VGA display.
 *   INPUTS: blank_bit -- set to 1 to blank, 0 to unblank
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
static void
VGA_blank (int blank_bit)
{
    /*
     * Move blanking bit into position for VGA sequencer register
     * (index 1).
     */
    blank_bit = ((blank_bit & 1) << 5);

    asm volatile (
        "movb $0x01,%%al         /* Set sequencer index to 1. */       ;"
        "movw $0x03C4,%%dx                                             ;"
        "outb %%al,(%%dx)                                              ;"
        "incw %%dx                                                     ;"
        "inb (%%dx),%%al         /* Read old value.           */       ;"
        "andb $0xDF,%%al         /* Calculate new value.      */       ;"
        "orl %0,%%eax                                                  ;"
        "outb %%al,(%%dx)        /* Write new value.          */       ;"
        "movw $0x03DA,%%dx       /* Enable display (0x20->P[0x3C0]) */ ;"
        "inb (%%dx),%%al         /* Set attr reg state to index. */    ;"
        "movw $0x03C0,%%dx       /* Write index 0x20 to enable. */     ;"
        "movb $0x20,%%al                                               ;"
        "outb %%al,(%%dx)                                               "
        : : "g" (blank_bit) : "eax", "edx", "memory");
}


/*
 * set_seq_regs_and_reset
 *   DESCRIPTION: Set VGA sequencer registers and miscellaneous output
 *                register; array of registers should force a reset of
 *                the VGA sequencer, which is restored to normal operation
 *                after a brief delay.
 *   INPUTS: table -- table of sequencer register values to use
 *           val -- value to which miscellaneous output register should be set
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
static void
set_seq_regs_and_reset (unsigned short table[NUM_SEQUENCER_REGS],
                        unsigned char val)
{
    /*
     * Dump table of values to sequencer registers.  Includes forced reset
     * as well as video blanking.
     */
    REP_OUTSW (0x03C4, table, NUM_SEQUENCER_REGS);

    /* Delay a bit... */
    {volatile int ii; for (ii = 0; ii < 10000; ii++);}

    /* Set VGA miscellaneous output register. */
    OUTB (0x03C2, val);

    /* Turn sequencer on (array values above should always force reset). */
    OUTW (0x03C4,0x0300);
}


/*
 * set_CRTC_registers
 *   DESCRIPTION: Set VGA cathode ray tube controller (CRTC) registers.
 *   INPUTS: table -- table of CRTC register values to use
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
static void
set_CRTC_registers (unsigned short table[NUM_CRTC_REGS])
{
    /* clear protection bit to enable write access to first few registers */
    OUTW (0x03D4, 0x0011);
    REP_OUTSW (0x03D4, table, NUM_CRTC_REGS);
}


/*
 * set_attr_registers
 *   DESCRIPTION: Set VGA attribute registers.  Attribute registers use
 *                a single port and are thus written as a sequence of bytes
 *                rather than a sequence of words.
 *   INPUTS: table -- table of attribute register values to use
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
static void
set_attr_registers (unsigned char table[NUM_ATTR_REGS * 2])
{
    /* Reset attribute register to write index next rather than data. */
    asm volatile (
        "inb (%%dx),%%al"
        : : "d" (0x03DA) : "eax", "memory");
    REP_OUTSB (0x03C0, table, NUM_ATTR_REGS * 2);
}


/*
 * set_graphics_registers
 *   DESCRIPTION: Set VGA graphics registers.
 *   INPUTS: table -- table of graphics register values to use
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
static void
set_graphics_registers (unsigned short table[NUM_GRAPHICS_REGS])
{
    REP_OUTSW (0x03CE, table, NUM_GRAPHICS_REGS);
}

static void change_pallette(short pallete_index, unsigned char r, unsigned char g, unsigned char b) {
    OUTB(0x03C8, pallete_index);
    unsigned char RGB[3] = {r, g, b};
    REP_OUTSB(0x03C9, RGB, 3);
}


/*
 * fill_palette
 *   DESCRIPTION: Fill VGA palette with necessary colors for the maze game.
 *                Only the first 64 (of 256) colors are written.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: changes the first 64 palette colors
 */
static void
fill_palette ()
{
    /* 6-bit RGB (red, green, blue) values for first 64 colors */
    static unsigned char palette_RGB[71][3] = {
        {0x00, 0x00, 0x00}, {0x00, 0x00, 0x2A},   /* palette 0x00 - 0x0F    */
        {0x00, 0x2A, 0x00}, {0x00, 0x2A, 0x2A},   /* basic VGA colors       */
        {0x2A, 0x00, 0x00}, {0x2A, 0x00, 0x2A},
        {0x2A, 0x15, 0x00}, {0x2A, 0x2A, 0x2A},
        {0x15, 0x15, 0x15}, {0x15, 0x15, 0x3F},
        {0x15, 0x3F, 0x15}, {0x15, 0x3F, 0x3F},
        {0x3F, 0x15, 0x15}, {0x3F, 0x15, 0x3F},
        {0x3F, 0x3F, 0x15}, {0x3F, 0x3F, 0x3F},
        {0x00, 0x00, 0x00}, {0x05, 0x05, 0x05},   /* palette 0x10 - 0x1F    */
        {0x08, 0x08, 0x08}, {0x0B, 0x0B, 0x0B},   /* VGA grey scale         */
        {0x0E, 0x0E, 0x0E}, {0x11, 0x11, 0x11},
        {0x14, 0x14, 0x14}, {0x18, 0x18, 0x18},
        {0x1C, 0x1C, 0x1C}, {0x20, 0x20, 0x20},
        {0x24, 0x24, 0x24}, {0x28, 0x28, 0x28},
        {0x2D, 0x2D, 0x2D}, {0x32, 0x32, 0x32},
        {0x38, 0x38, 0x38}, {0x3F, 0x3F, 0x3F},
        {0x3F, 0x3F, 0x3F}, {0x3F, 0x3F, 0x3F},   /* palette 0x20 - 0x2F    */
        {0x00, 0x00, 0x3F}, {0xFF, 0xFF, 0xFF},   /* wall and player colors */
        {0x28, 0x28, 0x28}, {0x00, 0x00, 0x00},
        {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00},
        {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00},
        {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00},
        {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00},
        {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00},
        {0x10, 0x08, 0x00}, {0x18, 0x0C, 0x00},   /* palette 0x30 - 0x3F    */
        {0x20, 0x10, 0x00}, {0x28, 0x14, 0x00},   /* browns for maze floor  */
        {0x30, 0x18, 0x00}, {0x38, 0x1C, 0x00},
        {0x3F, 0x20, 0x00}, {0x3F, 0x20, 0x10},
        {0x20, 0x18, 0x10}, {0x28, 0x1C, 0x10},
        {0x3F, 0x20, 0x10}, {0x38, 0x24, 0x10},
        {0x3F, 0x28, 0x10}, {0x3F, 0x2C, 0x10},
        {0x3F, 0x30, 0x10}, {0x3F, 0x20, 0x10},
        {0x27, 0x23, 0x1F}, {0x2B, 0x25, 0x1F},   /* palett 0x40 - 0x4F      */
        {0x2F, 0x25, 0x1F}, {0x33, 0x29, 0x1D},   /* Transparent colors, starting with maze floor */
        {0x37, 0x2B, 0x1F}, {0x1F, 0x1F, 0x34},   /* 0x45 is maze wall transparent */
        {0x2D, 0x2D, 0x2D}                        /* 0x46 is fog transparent */
    };

    /* Start writing at color 0. */
    OUTB (0x03C8, 0x00);

    /* Write all 64 colors from array. */
    REP_OUTSB (0x03C9, palette_RGB, 71 * 3);
}


/*
 * write_font_data
 *   DESCRIPTION: Copy font data into VGA memory, changing and restoring
 *                VGA register values in order to do so.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: leaves VGA registers in final text mode state
 */
static void
write_font_data ()
{
    int i;                /* loop index over characters                   */
    int j;                /* loop index over font bytes within characters */
    unsigned char* fonts; /* pointer into video memory                    */

    /* Prepare VGA to write font data into video memory. */
    OUTW (0x3C4, 0x0402);
    OUTW (0x3C4, 0x0704);
    OUTW (0x3CE, 0x0005);
    OUTW (0x3CE, 0x0406);
    OUTW (0x3CE, 0x0204);

    /* Copy font data from array into video memory. */
    for (i = 0, fonts = mem_image; i < 256; i++) {
        for (j = 0; j < 16; j++)
            fonts[j] = font_data[i][j];
        fonts += 32; /* skip 16 bytes between characters */
    }

    /* Prepare VGA for text mode. */
    OUTW (0x3C4, 0x0302);
    OUTW (0x3C4, 0x0304);
    OUTW (0x3CE, 0x1005);
    OUTW (0x3CE, 0x0E06);
    OUTW (0x3CE, 0x0004);
}


/*
 * set_text_mode_3
 *   DESCRIPTION: Put VGA into text mode 3 (color text).
 *   INPUTS: clear_scr -- if non-zero, clear screens; otherwise, do not
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: may clear screens; writes font data to video memory
 */
static void
set_text_mode_3 (int clear_scr)
{
    unsigned long* txt_scr; /* pointer to text screens in video memory */
    int i;                  /* loop over text screen words             */

    VGA_blank (1);                               /* blank the screen        */
    /*
     * The value here had been changed to 0x63, but seems to work
     * fine in QEMU (and VirtualPC, where I got it) with the 0x04
     * bit set (VGA_MIS_DCLK_28322_720).
     */
    set_seq_regs_and_reset (text_seq, 0x67);     /* sequencer registers     */
    set_CRTC_registers (text_CRTC);              /* CRT control registers   */
    set_attr_registers (text_attr);              /* attribute registers     */
    set_graphics_registers (text_graphics);      /* graphics registers      */
    fill_palette ();                 /* palette colors          */
    if (clear_scr) {                 /* clear screens if needed */
        txt_scr = (unsigned long*)(mem_image + 0x18000);
        for (i = 0; i < 8192; i++)
            *txt_scr++ = 0x07200720;
    }
    write_font_data ();                          /* copy fonts to video mem */
    VGA_blank (0);                     /* unblank the screen      */
}


/*
 * copy_image
 *   DESCRIPTION: Copy one plane of a screen from the build buffer to the
 *                video memory.
 *   INPUTS: img -- a pointer to a single screen plane in the build buffer
 *           scr_addr -- the destination offset in video memory
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: copies a plane from the build buffer to video memory
 */
static void
copy_image (unsigned char* img, unsigned short scr_addr)
{
    /*
     * memcpy is actually probably good enough here, and is usually
     * implemented using ISA-specific features like those below,
     * but the code here provides an example of x86 string moves
     */
    asm volatile (
        "cld                                                 ;"
        "movl $14560,%%ecx                                   ;" /* 14560 is SCROLL_Y_DIM * SCROLL_X_DIM / 4
                                                                   or the size of a plane in the build buffer */
        "rep movsb    # copy ECX bytes from M[ESI] to M[EDI]  "
        : /* no outputs */
        : "S" (img), "D" (mem_image + scr_addr)
        : "eax", "ecx", "memory"
        );
}

/*
 * copy_image
 *   DESCRIPTION: Copy one plane of the statusbar from the sb_build buffer to video memory.
 *   INPUTS: img -- a pointer to a single plane in sb_build buffer
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: copies a plane from the sb_build buffer to video memory
 */
static void
copy_image_statusbar (unsigned char* img)
{
    /*
     * memcpy is actually probably good enough here, and is usually
     * implemented using ISA-specific features like those below,
     * but the code here provides an example of x86 string moves
     */
    asm volatile (
        "cld                                                 ;"
        "movl $1440,  %%ecx                                   ;" /* 1440 is STATUSBAR_PLANE_SIZE */
        "rep movsb    # copy ECX bytes from M[ESI] to M[EDI]  "
        : /* no outputs */
        : "S" (img), "D" (mem_image + 0x0)                       /* The statusbar is located at the very beginning of modex memory */
        : "eax", "ecx", "memory"
        );
}

#if defined(TEXT_RESTORE_PROGRAM)

/*
 * main -- for the "tr" program
 *   DESCRIPTION: Put the VGA into text mode 3 without clearing the screens,
 *                which serves as a useful debugging tool when trying to
 *                debug programs that rely on having the VGA in mode X for
 *                normal operation.  Writes font data to video memory.
 *   INPUTS: none (command line arguments are ignored)
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, 3 in panic scenarios
 */
/* int */
/* main () */
/* { */
/*     if (open_memory_and_ports () == -1) */
/*         return 3; */

/*     set_text_mode_3 (1); */

/*     /\* Unmap video memory. *\/ */
/*     (void)munmap (mem_image, VID_MEM_SIZE); */

/*     /\* Return success. *\/ */
/*     return 0; */
/* } */

#endif
