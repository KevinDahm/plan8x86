#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

uint32_t strlen(const uint8_t* s)
{
    uint32_t len;

    for (len = 0; '\0' != *s; s++, len++);
    return len;
}

void strcpy(uint8_t* dst, const uint8_t* src)
{
    while ('\0' != (*dst++ = *src++));
}

void fdputs(int32_t fd, const uint8_t* s)
{
    (void)write (fd, s, strlen(s));
}

int32_t strcmp(const uint8_t* s1, const uint8_t* s2)
{
    while (*s1 == *s2) {
        if (*s1 == '\0')
            return 0;
        s1++;
        s2++;
    }
    return ((int32_t)*s1) - ((int32_t)*s2);
}

int32_t strncmp(const uint8_t* s1, const uint8_t* s2, uint32_t n)
{
    if (0 == n)
        return 0;
    while (*s1 == *s2) {
        if (*s1 == '\0' || --n == 0)
        return 0;
    s1++;
    s2++;
    }
    return ((int32_t)*s1) - ((int32_t)*s2);
}

/* Convert a number to its ASCII representation, with base "radix" */
uint8_t* itoa(uint32_t value, uint8_t* buf, int32_t radix)
{
        static int8_t lookup[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

        uint8_t *newbuf = buf;
        int32_t i;
        uint32_t newval = value;

        /* Special case for zero */
        if(value == 0) {
                buf[0]='0';
                buf[1]='\0';
                return buf;
        }

        /* Go through the number one place value at a time, and add the
         * correct digit to "newbuf".  We actually add characters to the
         * ASCII string from lowest place value to highest, which is the
         * opposite of how the number should be printed.  We'll reverse the
         * characters later. */
        while (newval > 0) {
                i = newval % radix;
                *newbuf = lookup[i];
                newbuf++;
                newval /= radix;
        }

        /* Add a terminating NULL */
        *newbuf = '\0';

        /* Reverse the string and return */
        return strrev(buf);
}



/* In-place string reversal */
uint8_t* strrev(uint8_t* s)
{
    register uint8_t tmp;
    register int32_t beg = 0;
    register int32_t end = strlen(s) - 1;

    if (end <= 0) {
        return s;
    }

    while (beg < end) {
        tmp = s[end];
        s[end] = s[beg];
        s[beg] = tmp;
        beg++;
        end--;
   }

   return s;
}

/*
 * void* memset(void* s, int32_t c, uint32_t n);
 *   Inputs: void* s = pointer to memory
 *            int32_t c = value to set memory to
 *            uint32_t n = number of bytes to set
 *   Return Value: new string
 *    Function: set n consecutive bytes of pointer s to value c
 */

void* memset(void* s, int32_t c, uint32_t n) {
    c &= 0xFF;
    asm volatile("                  \n\
            .memset_top:            \n\
            testl   %%ecx, %%ecx    \n\
            jz      .memset_done    \n\
            testl   $0x3, %%edi     \n\
            jz      .memset_aligned \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            subl    $1, %%ecx       \n\
            jmp     .memset_top     \n\
            .memset_aligned:        \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            movl    %%ecx, %%edx    \n\
            shrl    $2, %%ecx       \n\
            andl    $0x3, %%edx     \n\
            cld                     \n\
            rep     stosl           \n\
            .memset_bottom:         \n\
            testl   %%edx, %%edx    \n\
            jz      .memset_done    \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            subl    $1, %%edx       \n\
            jmp     .memset_bottom  \n\
            .memset_done:           \n\
            "
                 :
                 : "a"(c << 24 | c << 16 | c << 8 | c), "D"(s), "c"(n)
                 : "edx", "memory", "cc"
        );

    return s;
}

/*
 * void* memset_word(void* s, int32_t c, uint32_t n);
 *   Inputs: void* s = pointer to memory
 *            int32_t c = value to set memory to
 *            uint32_t n = number of bytes to set
 *   Return Value: new string
 *    Function: set lower 16 bits of n consecutive memory locations of pointer s to value c
 */

/* Optimized memset_word */
void* memset_word(void* s, int32_t c, uint32_t n) {
    asm volatile("                  \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            cld                     \n\
            rep     stosw           \n\
            "
                 :
                 : "a"(c), "D"(s), "c"(n)
                 : "edx", "memory", "cc"
        );

    return s;
}

/*
 * void* memset_dword(void* s, int32_t c, uint32_t n);
 *   Inputs: void* s = pointer to memory
 *            int32_t c = value to set memory to
 *            uint32_t n = number of bytes to set
 *   Return Value: new string
 *    Function: set n consecutive memory locations of pointer s to value c
 */

void* memset_dword(void* s, int32_t c, uint32_t n) {
    asm volatile("                  \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            cld                     \n\
            rep     stosl           \n\
            "
                 :
                 : "a"(c), "D"(s), "c"(n)
                 : "edx", "memory", "cc"
        );

    return s;
}

/*
 * void* memcpy(void* dest, const void* src, uint32_t n);
 *   Inputs: void* dest = destination of copy
 *            const void* src = source of copy
 *            uint32_t n = number of byets to copy
 *   Return Value: pointer to dest
 *    Function: copy n bytes of src to dest
 */

void* memcpy(void* dest, const void* src, uint32_t n) {
    asm volatile("                  \n\
            .memcpy_top:            \n\
            testl   %%ecx, %%ecx    \n\
            jz      .memcpy_done    \n\
            testl   $0x3, %%edi     \n\
            jz      .memcpy_aligned \n\
            movb    (%%esi), %%al   \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            addl    $1, %%esi       \n\
            subl    $1, %%ecx       \n\
            jmp     .memcpy_top     \n\
            .memcpy_aligned:        \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            movl    %%ecx, %%edx    \n\
            shrl    $2, %%ecx       \n\
            andl    $0x3, %%edx     \n\
            cld                     \n\
            rep     movsl           \n\
            .memcpy_bottom:         \n\
            testl   %%edx, %%edx    \n\
            jz      .memcpy_done    \n\
            movb    (%%esi), %%al   \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            addl    $1, %%esi       \n\
            subl    $1, %%edx       \n\
            jmp     .memcpy_bottom  \n\
            .memcpy_done:           \n\
            "
                 :
                 : "S"(src), "D"(dest), "c"(n)
                 : "eax", "edx", "memory", "cc"
        );

    return dest;
}

/*
 * void* memmove(void* dest, const void* src, uint32_t n);
 *   Inputs: void* dest = destination of move
 *            const void* src = source of move
 *            uint32_t n = number of byets to move
 *   Return Value: pointer to dest
 *    Function: move n bytes of src to dest
 */

/* Optimized memmove (used for overlapping memory areas) */
void* memmove(void* dest, const void* src, uint32_t n) {
    asm volatile("                  \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            cld                     \n\
            cmp     %%edi, %%esi    \n\
            jae     .memmove_go     \n\
            leal    -1(%%esi, %%ecx), %%esi    \n\
            leal    -1(%%edi, %%ecx), %%edi    \n\
            std                     \n\
            .memmove_go:            \n\
            rep     movsb           \n\
            "
                 :
                 : "D"(dest), "S"(src), "c"(n)
                 : "edx", "memory", "cc"
        );

    return dest;
}
