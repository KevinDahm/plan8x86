/* lib.h - Defines for useful library functions
 * vim:ts=4 noexpandtab
 */

#ifndef _LIB_H
#define _LIB_H

#include "types.h"
#include "schedule.h"
#include "page.h"
// Clears video memory for each terminal
void video_init();

// Standard printf
extern int32_t printf(int8_t *format, ...);

// Output a character to the console
void putc(uint8_t c);

//removes a character from the console
void removec();

//Output a string to the console
int32_t puts(int8_t *s);

//Convert a number to its ASCII representation, with base "radix"
int8_t *itoa(uint32_t value, int8_t* buf, int32_t radix);

//reverses a string
int8_t *strrev(int8_t* s);

//return length of string
uint32_t strlen(const int8_t* s);

//Clears videdo memory
void clear(void);

//Clears video memory, sets the color to blue, and resets the cursor
void blue_screen(void);

//displays the cursor at the current position
void update_cursor();

//Sets cursor to columnn x, row y
void set_cursor(uint32_t x, uint32_t y);

//Sets future writes to given color
void set_color(uint8_t col);

//Moves everything in the console up one row
void move_up();

//Switches the active terminal
void update_screen(uint32_t terminal);

//set n consecutive bytes of pointer s to value c
void* memset(void* s, int32_t c, uint32_t n);

//set lower 16 bits of n consecutive memory locations of pointer s to value c
void* memset_word(void* s, int32_t c, uint32_t n);

// set n consecutive memory locations of pointer s to value c
void* memset_dword(void* s, int32_t c, uint32_t n);

// copy n bytes of src to dest
void* memcpy(void* dest, const void* src, uint32_t n);

// move n bytes of src to dest
void* memmove(void* dest, const void* src, uint32_t n);

//compares string 1 and string 2 for equality
int32_t strncmp(const int8_t* s1, const int8_t* s2, uint32_t n);

// copy the source string into the destination string
int8_t* strcpy(int8_t* dest, const int8_t*src);

//copy n bytes of the source string into the destination string
int8_t* strncpy(int8_t* dest, const int8_t*src, uint32_t n);

//Increments all of video memory
void test_interrupts();

int8_t terminal_video[NUM_TERM][KB4] __attribute__((aligned (KB4)));

/* Port read functions */
/* Inb reads a byte and returns its value as a zero-extended 32-bit
 * unsigned int */
static inline uint32_t inb(port)
{
    uint32_t val;
    asm volatile("xorl %0, %0\n \
            inb   (%w1), %b0"
                 : "=a"(val)
                 : "d"(port)
                 : "memory" );
    return val;
}

/* Reads two bytes from two consecutive ports, starting at "port",
 * concatenates them little-endian style, and returns them zero-extended
 * */
static inline uint32_t inw(port)
{
    uint32_t val;
    asm volatile("xorl %0, %0\n   \
            inw   (%w1), %w0"
                 : "=a"(val)
                 : "d"(port)
                 : "memory" );
    return val;
}

/* Reads four bytes from four consecutive ports, starting at "port",
 * concatenates them little-endian style, and returns them */
static inline uint32_t inl(port)
{
    uint32_t val;
    asm volatile("inl   (%w1), %0"
                 : "=a"(val)
                 : "d"(port)
                 : "memory" );
    return val;
}

/* Writes a byte to a port */
#define outb(data, port)                        \
    do {                                        \
        asm volatile("outb  %b1, (%w0)"         \
                     :                          \
                     : "d" (port), "a" (data)   \
                     : "memory", "cc" );        \
    } while(0)

/* Writes two bytes to two consecutive ports */
#define outw(data, port)                        \
    do {                                        \
        asm volatile("outw  %w1, (%w0)"         \
                     :                          \
                     : "d" (port), "a" (data)   \
                     : "memory", "cc" );        \
    } while(0)

/* Writes four bytes to four consecutive ports */
#define outl(data, port)                        \
    do {                                        \
        asm volatile("outl  %l1, (%w0)"         \
                     :                          \
                     : "d" (port), "a" (data)   \
                     : "memory", "cc" );        \
    } while(0)

/* Clear interrupt flag - disables interrupts on this processor */
#define cli()                                   \
    do {                                        \
        asm volatile("cli"                      \
                     :                          \
                     :                          \
                     : "memory", "cc"           \
            );                                  \
    } while(0)

/* Save flags and then clear interrupt flag
 * Saves the EFLAGS register into the variable "flags", and then
 * disables interrupts on this processor */
#define cli_and_save(flags)                     \
    do {                                        \
        asm volatile("pushfl        \n      \
            popl %0         \n      \
            cli"                                \
                     : "=r"(flags)              \
                     :                          \
                     : "memory", "cc"           \
            );                                  \
    } while(0)

/* Set interrupt flag - enable interrupts on this processor */
#define sti()                                   \
    do {                                        \
        asm volatile("sti"                      \
                     :                          \
                     :                          \
                     : "memory", "cc"           \
            );                                  \
    } while(0)

/* Restore flags
 * Puts the value in "flags" into the EFLAGS register.  Most often used
 * after a cli_and_save_flags(flags) */
#define restore_flags(flags)                    \
    do {                                        \
        asm volatile("pushl %0      \n      \
            popfl"                              \
                     :                          \
                     : "r"(flags)               \
                     : "memory", "cc"           \
            );                                  \
    } while(0)

#endif /* _LIB_H */
