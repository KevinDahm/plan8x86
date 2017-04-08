#include "task.h"
#include "lib.h"

void create_init() {
    tasks[0].status = TASK_RUNNING;

    memset(tasks[0].page_directory_tables, 2, DIR_SIZE * 4);
    memset(tasks[0].page_tables, 2, DIR_SIZE * 4);

    page_dir_kb_entry_t* page_table_entry = (page_dir_kb_entry_t*)tasks[0].page_directory_tables;
    page_table_entry->addr = (uint32_t)tasks[0].page_tables >> 12;//Lose lower 12 bits (keep 20 high bits)
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
        (page_table_kb_entry_t*)(tasks[0].page_tables + (VIDEO >> 12));
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

    page_dir_mb_entry_t* kernel_entry = (page_dir_mb_entry_t*)(tasks[0].page_directory_tables+1);
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

    int task;
    for (task = 1; task < NUM_TASKS; task++) {
        memset(tasks[task].page_directory_tables, 2, DIR_SIZE * 4);
        memset(tasks[task].page_tables, 2, DIR_SIZE * 4);

        page_dir_kb_entry_t* page_table_entry = (page_dir_kb_entry_t*)tasks[task].page_directory_tables;
        page_table_entry->addr = (uint32_t)tasks[task].page_tables >> 12;//Lose lower 12 bits (keep 20 high bits)
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
            (page_table_kb_entry_t*)(tasks[task].page_tables + (VIDEO >> 12));
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

        page_dir_mb_entry_t* kernel_entry = (page_dir_mb_entry_t*)(tasks[task].page_directory_tables+1);
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

        page_dir_mb_entry_t* task1_entry = (page_dir_mb_entry_t*)(tasks[task].page_directory_tables+32); // 128MB virtual
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
}
