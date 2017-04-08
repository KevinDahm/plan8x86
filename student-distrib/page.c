#include "page.h"
#include "lib.h"
#include "task.h"

/* init_paging
 * Description: Sets flags and moves directory address to CR3
 * Inputs:      None
 * Outputs:     None
 * Ret Value:   None
 * Side Effect: Paging is enabled
 */
void init_paging(){
    asm volatile("              \n\
    movl    %%cr4, %%eax        \n\
    orl     $0x00000010, %%eax  \n\
    movl    %%eax, %%cr4        \n\
                                \n\
    movl    %%cr0, %%eax        \n\
    orl     $0x80000001, %%eax  \n\
    movl    %%eax, %%cr0        \n\
    "
                 : /* no outputs */
                 : /* no inputs  */
        );
}

void switch_page_directory(int pd) {
    asm volatile("              \n\
    movl    %0, %%cr3           \n\
    "
                 : /* no outputs */
                 : "a"(tasks[pd].page_directory_tables)
        );
}

