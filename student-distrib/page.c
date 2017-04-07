#include "page.h"
#include "lib.h"

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
                 : "a"(page_directory_tables[pd])
        );
}


/* clear_tables
 * Description: Sets tables to write mode but not present
 * Inputs:      None
 * Outputs:     None
 * Ret Value:   None
 * Side Effect: Tables are set to empty
 */
void clear_tables() {
    // Write 2 to all pages, enableing read/write
    memset(page_directory_tables, 2, NUM_DIRS * DIR_SIZE * 4);
    memset(page_tables, 2, NUM_DIRS * DIR_SIZE * 4);
}


/* create_entries
 * Description: Initializes kernel and video memory in paging tables
 * Inputs:      None
 * Outputs:     None
 * Ret Value:   None
 * Side Effect: Entries are made in the paging tables
 */
void create_entries() {
    // KERNEL
    {
        page_dir_kb_entry_t* page_table_entry = (page_dir_kb_entry_t*)page_directory_tables[0];
        page_table_entry->addr = (uint32_t)page_tables[0] >> 12;//Lose lower 12 bits (keep 20 high bits)
        page_table_entry->avail = 0;
        page_table_entry->global = 0;
        page_table_entry->pageSize = 0;     //0 for kb
        page_table_entry->reserved = 0;
        page_table_entry->accessed = 0;
        page_table_entry->cacheDisabled = 0;
        page_table_entry->writeThrough = 1; //1 for fun
        page_table_entry->userSupervisor = 0;
        page_table_entry->readWrite = 1;    //Write enabled
        page_table_entry->present = 1;

        page_table_kb_entry_t* video_entry =
            (page_table_kb_entry_t*)(page_tables[0] + (VIDEO >> 12));
        video_entry->addr = VIDEO >> 12;    //Lose lower 12 bits (keep 20 high bits)
        video_entry->avail = 0;
        video_entry->global = 0;
        video_entry->pgTblAttIdx = 0;
        video_entry->dirty = 0;
        video_entry->accessed = 0;
        video_entry->cacheDisabled = 0;
        video_entry->writeThrough = 1;  //1 for fun
        video_entry->userSupervisor = 0;
        video_entry->readWrite = 1;     //Write enabled
        video_entry->present = 1;

        page_dir_mb_entry_t* kernel_entry = (page_dir_mb_entry_t*)(page_directory_tables[0]+1);
        kernel_entry->addr = KERNEL >> 22;  //Lose lower 22 bits (keep 10 high bits)
        kernel_entry->reserved = 0;
        kernel_entry->pgTblAttIdx = 0;
        kernel_entry->avail = 0;
        kernel_entry->global = 1;       //1 for global buffer usage
        kernel_entry->pageSize = 1;     //1 for mb
        kernel_entry->reserved = 0;
        kernel_entry->accessed = 0;
        kernel_entry->cacheDisabled = 0;
        kernel_entry->writeThrough = 1; //1 or fun
        kernel_entry->userSupervisor = 0;
        kernel_entry->readWrite = 1;    //Write enabled
        kernel_entry->present = 1;
    }
    // TASK 1
    int task;
    for (task = 1; task <= 2; task++) {
        page_dir_kb_entry_t* page_table_entry = (page_dir_kb_entry_t*)page_directory_tables[task];
        page_table_entry->addr = (uint32_t)page_tables[task] >> 12;//Lose lower 12 bits (keep 20 high bits)
        page_table_entry->avail = 0;
        page_table_entry->global = 0;
        page_table_entry->pageSize = 0;     //0 for kb
        page_table_entry->reserved = 0;
        page_table_entry->accessed = 0;
        page_table_entry->cacheDisabled = 0;
        page_table_entry->writeThrough = 1; //1 for fun
        page_table_entry->userSupervisor = 0;
        page_table_entry->readWrite = 1;    //Write enabled
        page_table_entry->present = 1;

        page_table_kb_entry_t* video_entry =
            (page_table_kb_entry_t*)(page_tables[task] + (VIDEO >> 12));
        video_entry->addr = VIDEO >> 12;    //Lose lower 12 bits (keep 20 high bits)
        video_entry->avail = 0;
        video_entry->global = 0;
        video_entry->pgTblAttIdx = 0;
        video_entry->dirty = 0;
        video_entry->accessed = 0;
        video_entry->cacheDisabled = 0;
        video_entry->writeThrough = 1;  //1 for fun
        video_entry->userSupervisor = 0;
        video_entry->readWrite = 1;     //Write enabled
        video_entry->present = 1;

        page_dir_mb_entry_t* kernel_entry = (page_dir_mb_entry_t*)(page_directory_tables[task]+1);
        kernel_entry->addr = KERNEL >> 22;  //Lose lower 22 bits (keep 10 high bits)
        kernel_entry->reserved = 0;
        kernel_entry->pgTblAttIdx = 0;
        kernel_entry->avail = 0;
        kernel_entry->global = 1;       //1 for global buffer usage
        kernel_entry->pageSize = 1;     //1 for mb
        kernel_entry->reserved = 0;
        kernel_entry->accessed = 0;
        kernel_entry->cacheDisabled = 0;
        kernel_entry->writeThrough = 1; //1 or fun
        kernel_entry->userSupervisor = 1;
        kernel_entry->readWrite = 1;    //Write enabled
        kernel_entry->present = 1;

        page_dir_mb_entry_t* task1_entry = (page_dir_mb_entry_t*)(page_directory_tables[task]+32); // 128MB virtual
        task1_entry->addr = (0x00400000 * (task + 1)) >> 22;  // 4MB for each task starting at physical address 8MB.
        task1_entry->reserved = 0;
        task1_entry->pgTblAttIdx = 0;
        task1_entry->avail = 0;
        task1_entry->global = 1;       //1 for global buffer usage
        task1_entry->pageSize = 1;     //1 for mb
        task1_entry->reserved = 0;
        task1_entry->accessed = 0;
        task1_entry->cacheDisabled = 0;
        task1_entry->writeThrough = 1; //1 or fun
        task1_entry->userSupervisor = 1; // TODO: set to 1 for user-level 3
        task1_entry->readWrite = 1;    //Write enabled
        task1_entry->present = 1;
    }
}
