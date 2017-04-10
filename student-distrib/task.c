#include "task.h"
#include "lib.h"
#include "page.h"

void create_init() {
    tasks[0] = &init_pcb;
    tasks[0]->status = TASK_RUNNING;
    tasks[0]->page_directory = page_directory_tables[0];
    tasks[0]->kernel_vid_table = page_tables[0][0];

    memset(tasks[0]->page_directory, 2, DIR_SIZE * 4);
    memset(tasks[0]->kernel_vid_table, 2, DIR_SIZE * 4);

    page_dir_kb_entry_t* kernel_vid_table = (page_dir_kb_entry_t*)tasks[0]->page_directory;
    kernel_vid_table->addr = (uint32_t)tasks[0]->kernel_vid_table >> 12;//Lose lower 12 bits (keep 20 high bits)
    kernel_vid_table->avail = 0;
    kernel_vid_table->global = 0;
    kernel_vid_table->pageSize = 0;     //0 for kb
    kernel_vid_table->reserved = 0;
    kernel_vid_table->accessed = 0;
    kernel_vid_table->cacheDisabled = 0;
    kernel_vid_table->writeThrough = 1; //1 for fun
    kernel_vid_table->userSupervisor = 0;
    kernel_vid_table->readWrite = 1;    //Write enabled
    kernel_vid_table->present = 1;

    page_table_kb_entry_t* kernel_vid_entry =
        (page_table_kb_entry_t*)(tasks[0]->kernel_vid_table + (VIDEO >> 12));
    kernel_vid_entry->addr = VIDEO >> 12;    //Lose lower 12 bits (keep 20 high bits)
    kernel_vid_entry->avail = 0;
    kernel_vid_entry->global = 0;
    kernel_vid_entry->pgTblAttIdx = 0;
    kernel_vid_entry->dirty = 0;
    kernel_vid_entry->accessed = 0;
    kernel_vid_entry->cacheDisabled = 0;
    kernel_vid_entry->writeThrough = 1;  //1 for fun
    kernel_vid_entry->userSupervisor = 0;
    kernel_vid_entry->readWrite = 1;     //Write enabled
    kernel_vid_entry->present = 1;

    page_dir_mb_entry_t* kernel_entry = (page_dir_mb_entry_t*)(tasks[0]->page_directory+1);
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

    switch_page_directory(0);

    uint32_t task;
    for (task = 1; task < NUM_TASKS; task++) {
        tasks[task] = (pcb_t *)((KERNEL + MB4) - (task * PER_TASK_KERNEL_STACK_SIZE));
        memset(tasks[task], 0, sizeof(pcb_t));
        tasks[task]->kernel_esp = (KERNEL + MB4) - ((task - 1) * PER_TASK_KERNEL_STACK_SIZE);
        tasks[task]->status = TASK_EMPTY;

        tasks[task]->page_directory = page_directory_tables[task];
        tasks[task]->kernel_vid_table = page_tables[task][0];
        tasks[task]->usr_vid_table = page_tables[task][1];

        memset(tasks[task]->page_directory, 2, DIR_SIZE * 4);
        memset(tasks[task]->kernel_vid_table, 2, DIR_SIZE * 4);
        memset(tasks[task]->usr_vid_table, 2, DIR_SIZE * 4);

        page_dir_kb_entry_t* kernel_vid_table = (page_dir_kb_entry_t*)tasks[task]->page_directory;
        kernel_vid_table->addr = (uint32_t)tasks[task]->kernel_vid_table >> 12;//Lose lower 12 bits (keep 20 high bits)
        kernel_vid_table->avail = 0;
        kernel_vid_table->global = 0;
        kernel_vid_table->pageSize = 0;     //0 for kb
        kernel_vid_table->reserved = 0;
        kernel_vid_table->accessed = 0;
        kernel_vid_table->cacheDisabled = 0;
        kernel_vid_table->writeThrough = 1; //1 for fun
        kernel_vid_table->userSupervisor = 0;
        kernel_vid_table->readWrite = 1;    //Write enabled
        kernel_vid_table->present = 1;

        page_table_kb_entry_t* kernel_vid_entry =
            (page_table_kb_entry_t*)(tasks[task]->kernel_vid_table + (VIDEO >> 12));
        kernel_vid_entry->addr = VIDEO >> 12;    //Lose lower 12 bits (keep 20 high bits)
        kernel_vid_entry->avail = 0;
        kernel_vid_entry->global = 0;
        kernel_vid_entry->pgTblAttIdx = 0;
        kernel_vid_entry->dirty = 0;
        kernel_vid_entry->accessed = 0;
        kernel_vid_entry->cacheDisabled = 0;
        kernel_vid_entry->writeThrough = 1;  //1 for fun
        kernel_vid_entry->userSupervisor = 0;
        kernel_vid_entry->readWrite = 1;     //Write enabled
        kernel_vid_entry->present = 1;

        page_dir_kb_entry_t* usr_vid_table = (page_dir_kb_entry_t*)tasks[task]->page_directory + 33; // 132MB virtual
        usr_vid_table->addr = (uint32_t)tasks[task]->usr_vid_table >> 12;//Lose lower 12 bits (keep 20 high bits)
        usr_vid_table->avail = 0;
        usr_vid_table->global = 0;
        usr_vid_table->pageSize = 0;     //0 for kb
        usr_vid_table->reserved = 0;
        usr_vid_table->accessed = 0;
        usr_vid_table->cacheDisabled = 0;
        usr_vid_table->writeThrough = 1; //1 for fun
        usr_vid_table->userSupervisor = 1;
        usr_vid_table->readWrite = 1;    //Write enabled
        usr_vid_table->present = 1;

        page_table_kb_entry_t* usr_vid_entry =
            (page_table_kb_entry_t*)(tasks[task]->usr_vid_table);
        usr_vid_entry->addr = VIDEO >> 12;    //Lose lower 12 bits (keep 20 high bits)
        usr_vid_entry->avail = 0;
        usr_vid_entry->global = 0;
        usr_vid_entry->pgTblAttIdx = 0;
        usr_vid_entry->dirty = 0;
        usr_vid_entry->accessed = 0;
        usr_vid_entry->cacheDisabled = 0;
        usr_vid_entry->writeThrough = 1;  //1 for fun
        usr_vid_entry->userSupervisor = 1;
        usr_vid_entry->readWrite = 1;     //Write enabled
        usr_vid_entry->present = 1;

        page_dir_mb_entry_t* kernel_entry = (page_dir_mb_entry_t*)(tasks[task]->page_directory+1);
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

        page_dir_mb_entry_t* task1_entry = (page_dir_mb_entry_t*)(tasks[task]->page_directory+32); // 128MB virtual
        task1_entry->addr = (MB4 * (task + 1)) >> 22;  // 4MB for each task starting at physical address 8MB.
        task1_entry->reserved = 0;
        task1_entry->pgTblAttIdx = 0;
        task1_entry->avail = 0;
        task1_entry->global = 1;       //1 for global buffer usage
        task1_entry->pageSize = 1;     //1 for mb
        task1_entry->reserved = 0;
        task1_entry->accessed = 0;
        task1_entry->cacheDisabled = 0;
        task1_entry->writeThrough = 1; //1 or fun
        task1_entry->userSupervisor = 1;
        task1_entry->readWrite = 1;    //Write enabled
        task1_entry->present = 1;
    }

    cur_task = 0;
}
