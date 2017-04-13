#include "task.h"
#include "lib.h"
#include "page.h"

/* setup_kernel_mem
 * Description: Sets up a 4KB page mapping to video memory
 * Input:  dir - A pointer to a page directory entry to fill out
 *         table - A pointer to a page table entry to fill out
 *         priv - 0 for kernel, 1 for user
 * Output: none
 * Side Effects: Writes to *dir and *table
 */
void setup_vid(uint32_t *dir, uint32_t *table, uint32_t priv) {
    page_dir_kb_entry_t* vid_table = (page_dir_kb_entry_t*)dir;
    vid_table->addr = (uint32_t)table >> 12;//Lose lower 12 bits (keep 20 high bits)
    vid_table->avail = 0;
    vid_table->global = 0;
    vid_table->pageSize = 0;     //0 for kb
    vid_table->reserved = 0;
    vid_table->accessed = 0;
    vid_table->cacheDisabled = 0;
    vid_table->writeThrough = 1; //1 for fun
    vid_table->userSupervisor = priv;
    vid_table->readWrite = 1;    //Write enabled
    vid_table->present = 1;

    page_table_kb_entry_t* vid_entry;
    if (priv == 0) {
        vid_entry = (page_table_kb_entry_t*)(table + (VIDEO >> 12));
    } else {
        vid_entry = (page_table_kb_entry_t*)(table);
    }
    vid_entry->addr = VIDEO >> 12;    //Lose lower 12 bits (keep 20 high bits)
    vid_entry->avail = 0;
    vid_entry->global = 0;
    vid_entry->pgTblAttIdx = 0;
    vid_entry->dirty = 0;
    vid_entry->accessed = 0;
    vid_entry->cacheDisabled = 0;
    vid_entry->writeThrough = 1;  //1 for fun
    vid_entry->userSupervisor = priv;
    vid_entry->readWrite = 1;     //Write enabled
    vid_entry->present = 1;
}

/* setup_kernel_mem
 * Description: Fills in a 4MB page directory entry at dir (which provides the virtual address) and maps it to physical address 4MB
 * Input:  dir - A pointer to a page directory entry to fill out
 * Output: none
 * Side Effects: Writes to *dir
 */
void setup_kernel_mem(uint32_t *dir) {
    page_dir_mb_entry_t* kernel_entry = (page_dir_mb_entry_t*)dir;
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

/* setup_task_mem
 * Description: Fills in a 4MB page directory entry at dir (which provides the virtual address) and maps it to physical address
 *              given by 4MB * (task + 1)
 * Input:  dir - A pointer to a page directory entry to fill out
 *         task - Which task to create the pde for
 * Output: none
 * Side Effects: Writes to *dir
 */
void setup_task_mem(uint32_t *dir, uint32_t task) {
    page_dir_mb_entry_t* task_entry = (page_dir_mb_entry_t*)(dir);
    task_entry->addr = (MB4 * (task + 1)) >> 22;  // 4MB for each task starting at physical address 8MB.
    task_entry->reserved = 0;
    task_entry->pgTblAttIdx = 0;
    task_entry->avail = 0;
    task_entry->global = 1;       //1 for global buffer usage
    task_entry->pageSize = 1;     //1 for mb
    task_entry->reserved = 0;
    task_entry->accessed = 0;
    task_entry->cacheDisabled = 0;
    task_entry->writeThrough = 1; //1 or fun
    task_entry->userSupervisor = 1;
    task_entry->readWrite = 1;    //Write enabled
    task_entry->present = 1;
}

/* create_init
 * Description: Initializes paging for the kernel and each user task and sets up default values for each task
 * Input:  none
 * Output: none
 * Side Effects: Fills the tasks array.
 */
void create_init() {
    tasks[0] = (pcb_t *)((KERNEL + MB4) - (PER_TASK_KERNEL_STACK_SIZE));
    memset(tasks[0], 0, sizeof(pcb_t));
    tasks[0]->kernel_esp = (KERNEL + MB4);
    tasks[0]->status = TASK_RUNNING;
    tasks[0]->page_directory = page_directory_tables[0];
    tasks[0]->kernel_vid_table = page_tables[0][0];

    memset(tasks[0]->page_directory, 2, DIR_SIZE * 4);
    memset(tasks[0]->kernel_vid_table, 2, DIR_SIZE * 4);

    setup_vid(tasks[0]->page_directory, tasks[0]->kernel_vid_table, 0);

    setup_kernel_mem(tasks[0]->page_directory + 1);

    uint32_t task;
    for (task = 1; task < NUM_TASKS; task++) {
        tasks[task] = (pcb_t *)((KERNEL + MB4) - ((task + 1) * PER_TASK_KERNEL_STACK_SIZE));
        memset(tasks[task], 0, sizeof(pcb_t));
        tasks[task]->kernel_esp = (KERNEL + MB4) - (task * PER_TASK_KERNEL_STACK_SIZE);
        tasks[task]->status = TASK_EMPTY;

        tasks[task]->page_directory = page_directory_tables[task];
        tasks[task]->kernel_vid_table = page_tables[task][0];
        tasks[task]->usr_vid_table = page_tables[task][1];

        memset(tasks[task]->page_directory, 2, DIR_SIZE * 4);
        memset(tasks[task]->kernel_vid_table, 2, DIR_SIZE * 4);
        memset(tasks[task]->usr_vid_table, 2, DIR_SIZE * 4);

        setup_vid(tasks[task]->page_directory, tasks[task]->kernel_vid_table, 0);
        setup_vid(tasks[task]->page_directory + 33, tasks[task]->usr_vid_table, 1);

        // 1 * 4MB for virtual address of 4MB
        setup_kernel_mem(tasks[task]->page_directory + 1);

        // 32 * 4MB for virtual address of 128MB
        setup_task_mem(tasks[task]->page_directory + 32, task);
    }

    cur_task = 0;
}
