#include "task.h"
#include "lib.h"
#include "page.h"
#include "rtc.h"

uint8_t cur_task = INIT;

//stub functions - default for file_ops
int32_t default_open(const int8_t *buf) {
    return -1;
}

int32_t default_close(int32_t fd) {
    return -1;
}

int32_t default_read(int32_t fd, void *buf, int32_t nbytes) {
    return -1;
}

int32_t default_write(int32_t fd, const void *buf, int32_t nbytes) {
    return -1;
}

int32_t default_stat(int32_t fd, void *buf, int32_t nbytes) {
    return -1;
}

/* void setup_vid(uint32_t *dir, uint32_t *table, uint32_t priv)
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
    vid_table->userSupervisor = 0;
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
    vid_entry->userSupervisor = 0;
    vid_entry->readWrite = 1;     //Write enabled
    vid_entry->present = 1;
}

/* void setup_kernel_mem(uint32_t *dir)
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

/* void setup_task_mem(uint32_t *dir, uint32_t task)
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

/* void create_init()
 * Description: Initializes paging for the kernel and each user task and sets up default values for each task
 * Input:  none
 * Output: none
 * Side Effects: Fills the tasks array.
 */
void create_init() {
    default_ops.open = default_open;
    default_ops.close = default_close;
    default_ops.read = default_read;
    default_ops.write = default_write;
    default_ops.stat = default_stat;

    memset(signal_handlers, 0, sizeof(signal_handlers));

    tasks[INIT] = &task_stacks[INIT].pcb;
    memset(tasks[INIT], 0, sizeof(pcb_t));
    tasks[INIT]->kernel_esp = (uint32_t)&task_stacks[INIT].stack_start;
    tasks[INIT]->rtc_base = MAX_RTC_FREQ;
    tasks[INIT]->rtc_counter = MAX_RTC_FREQ;

    tasks[INIT]->status = TASK_RUNNING;
    tasks[INIT]->page_directory = page_directory_tables[INIT];
    tasks[INIT]->kernel_vid_table = page_tables[INIT][0];

    memset(tasks[INIT]->page_directory, PAGE_RW, TABLE_SIZE);
    memset(tasks[INIT]->kernel_vid_table, PAGE_RW, TABLE_SIZE);

    setup_vid(tasks[INIT]->page_directory, tasks[INIT]->kernel_vid_table, 0);

    setup_kernel_mem(tasks[INIT]->page_directory + 1);

    tasks[INIT]->file_descs = file_desc_arrays[INIT];
    uint32_t file_i;
    for (file_i = 0; file_i < FILE_DESCS_LENGTH; file_i++) {
        tasks[INIT]->file_descs[file_i].ops = &default_ops;
    }

    uint32_t task;
    for (task = 1; task < NUM_TASKS; task++) {
        tasks[task] = &task_stacks[task].pcb;
        memset(tasks[task], 0, sizeof(pcb_t));
        tasks[task]->kernel_esp = (uint32_t)&task_stacks[task].stack_start;
        tasks[task]->status = TASK_EMPTY;

        tasks[task]->page_directory = page_directory_tables[task];
        tasks[task]->kernel_vid_table = page_tables[task][0];
        tasks[task]->usr_vid_table = page_tables[task][1];

        memset(tasks[task]->page_directory, PAGE_RW, TABLE_SIZE);
        memset(tasks[task]->kernel_vid_table, PAGE_RW, TABLE_SIZE);
        memset(tasks[task]->usr_vid_table, PAGE_RW, TABLE_SIZE);

        setup_vid(tasks[task]->page_directory, tasks[task]->kernel_vid_table, 0);
        setup_vid(tasks[task]->page_directory + 33, tasks[task]->usr_vid_table, 1);

        // 1 * 4MB for virtual address of 4MB
        setup_kernel_mem(tasks[task]->page_directory + 1);

        // 32 * 4MB for virtual address of 128MB
        setup_task_mem(tasks[task]->page_directory + 32, task);

        tasks[task]->file_descs = file_desc_arrays[task];
        uint32_t file_i;
        for (file_i = 0; file_i < FILE_DESCS_LENGTH; file_i++) {
            tasks[task]->file_descs[file_i].ops = &default_ops;
        }
    }

    cur_task = 0;
}
