#include "page.h"

void initPaging() {
/* Set PG flag in CR0 to 1 */
/* Set PSE flag in CR4 to 1 */
    asm volatile("          \n\
    movl    %%cr0, %%eax    \n\
    orl     $0x80, %%eax    \n\
    movl    %%eax, %%cr0    \n\
                            \n\
    movl    %%cr4, %%eax    \n\
    orl     $0x10, %%eax    \n\
    movl    %%eax, %%cr4    \n\
    "
                 : /* no outputs */
                 : /* no inputs */
                 : "%eax"
        );
}
