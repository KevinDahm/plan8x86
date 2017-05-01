#include "page.h"
#include "lib.h"
#include "task.h"

/* void init_paging()
 * Description: Sets flags for paging
 * Inputs:      None
 * Outputs:     None
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

/* void switch_page_directory(int task)
 * Description: moves directory address to CR3
 * Inputs:      task - task index to switch to
 * Outputs:     None
 * Side Effect: TLB flushed and paging swapped
 */
void switch_page_directory(int task) {
    asm volatile("              \n\
    movl    %0, %%cr3           \n\
    "
                 : /* no outputs */
                 : "a"(tasks[task]->page_directory)
        );
}
