#include "system_calls.h"
#include "filesystem.h"
#include "lib.h"
#include "rtc.h"
#include "kbd.h"
#include "terminal.h"
#include "page.h"
#include "entry.h"
#include "x86_desc.h"
#include "task.h"
#include "schedule.h"

bool backup_init_ebp = true;


uint32_t halt_status;
/* sys_halt
 * Description: Stops the process that called this and returns control to the proccess that ran sys_execute
 * Input:  status - The return value of the user process that called sys_halt
 * Output: returns status
 * Side Effects: resets the cur_task's pcb_t
 */
int32_t sys_halt(uint32_t status) {
    cli();
    int i;

    if (tasks[cur_task]->thread_status == 1 && tasks[tasks[cur_task]->parent]->thread_waiting == cur_task) {
        tasks[tasks[cur_task]->parent]->status = TASK_RUNNING;
        CLEAR_THREAD(tasks[cur_task]->parent, cur_task);
        tasks[cur_task]->status = TASK_EMPTY;
        goto sys_halt_return;
    } else if (tasks[cur_task]->thread_status == 1) {
        tasks[cur_task]->status = TASK_ZOMBIE;
        goto sys_halt_return;
    } else if (tasks[cur_task]->thread_status > 1) {
        i = 0;
        while (tasks[cur_task]->thread_status != 0) {
            if (tasks[cur_task]->thread_status & 1) {
                tasks[i]->status = TASK_EMPTY;
                tasks[i]->page_directory[34] = 2;
            }
            tasks[cur_task]->thread_status >>= 1;
            i++;
        }
        goto sys_halt_cleanup_files;
    } else {
        tasks[cur_task]->status = TASK_EMPTY;
        goto sys_halt_cleanup_files;
    }

sys_halt_cleanup_files:
    for (i = 0; i < FILE_DESCS_LENGTH; i++) {
        if (tasks[cur_task]->file_descs[i].flags != FD_CLEAR) {
            sys_close(i);
        }
    }

sys_halt_return:
    tasks[cur_task]->kernel_esp = (uint32_t)&task_stacks[cur_task].stack_start;
    uint32_t term = tasks[cur_task]->terminal;

    cur_task = tasks[cur_task]->parent;
    tasks[cur_task]->status = TASK_RUNNING;

    switch_page_directory(cur_task);

    memset(signal_handlers[cur_task], 0, sizeof(signal_handlers[cur_task]));

    if (cur_task == INIT) {
        tasks[INIT]->terminal = term;
        update_screen(term);
        sys_execute((uint8_t*)"shell");
    }

    uint32_t ebp = tasks[cur_task]->ebp;
    term_process[tasks[cur_task]->terminal] = cur_task;

    // Save the return value before moving the stack.
    // halt_status has to be static memory, it cannot be on the stack.
    halt_status = status;

    asm volatile("movl %0, %%ebp;" : : "r"(ebp));

    tss.esp0 = tasks[cur_task]->kernel_esp;

    asm volatile("movl %0, %%eax; leave; ret;" : : "r"(halt_status));

    return halt_status;
}

/* sys_execute
 * Description: Starts a new process specified by command
 * Input:  command - the process to start and any args to send to it
 * Output: -1 on error, 0 on success
 * Side Effects: modifies tasks
 */
int32_t sys_execute(const uint8_t* command) {
    cli();

    // Copy the command string for parsing later
    uint8_t com_str[strlen((int8_t*)command)];
    strcpy((int8_t*)com_str, (int8_t*)command);

    // Once INIT starts the initial shells, don't move it's base pointer or
    // sys_halt won't be able to restart an exited shell. This is because
    // if we do INIT won't return to it's hlt loop and instead will jump
    // into garbage from a random stack.
    if (cur_task != INIT || backup_init_ebp) {
        // Backup the ebp and kernel_esp of the parent task so that
        // after sys_halt is called by the child the parent can be resumed
        // in the correct location.
        uint32_t ebp;
        asm volatile("movl %%ebp, %0;" : "=r"(ebp) : );
        tasks[cur_task]->ebp = ebp;
        tasks[cur_task]->kernel_esp = ebp - 4;
    }

    // Find the first empty task to place the new one in.
    int32_t fd;
    uint8_t task_num;
    for (task_num = 1; task_num < NUM_TASKS; task_num++) {
        if (tasks[task_num]->status == TASK_EMPTY) {
            break;
        }
    }

    if (task_num >= NUM_TASKS) {
        return -1;
    }

    tasks[task_num] = &task_stacks[task_num].pcb;
    memset(tasks[task_num], 0, sizeof(pcb_t));
    tasks[task_num]->kernel_esp = (uint32_t)&task_stacks[task_num].stack_start;

    tasks[task_num]->parent = cur_task;
    cur_task = task_num;

    tasks[cur_task]->thread_status = 0;
    tasks[cur_task]->thread_waiting = 0;
    tasks[cur_task]->rtc_counter = 0;
    tasks[cur_task]->rtc_base = 512;
    tasks[cur_task]->pending_signals = 0;
    tasks[cur_task]->signal_mask = false;

    memset(signal_handlers[cur_task], 0, sizeof(signal_handlers[cur_task]));

    tasks[cur_task]->page_directory = page_directory_tables[cur_task];
    tasks[cur_task]->kernel_vid_table = page_tables[cur_task][0];
    tasks[cur_task]->usr_vid_table = page_tables[cur_task][1];

    memset(tasks[cur_task]->page_directory, 2, DIR_SIZE * 4);
    memset(tasks[cur_task]->kernel_vid_table, 2, DIR_SIZE * 4);
    memset(tasks[cur_task]->usr_vid_table, 2, DIR_SIZE * 4);

    setup_vid(tasks[cur_task]->page_directory, tasks[cur_task]->kernel_vid_table, 0);
    setup_vid(tasks[cur_task]->page_directory + 33, tasks[cur_task]->usr_vid_table, 1);

    // 1 * 4MB for virtual address of 4MB
    setup_kernel_mem(tasks[cur_task]->page_directory + 1);

    // 32 * 4MB for virtual address of 128MB
    setup_task_mem(tasks[cur_task]->page_directory + 32, cur_task);

    // Inherit the terminal from parent. Also sets the usr_vid_table page up depending
    // on whether terminal is focused.
    tasks[cur_task]->terminal = tasks[tasks[cur_task]->parent]->terminal;
    term_process[tasks[cur_task]->terminal] = cur_task;
    if (tasks[cur_task]->terminal == active) {
        page_table_kb_entry_t *usr_vid_table = (page_table_kb_entry_t *)tasks[cur_task]->usr_vid_table;
        usr_vid_table->addr = VIDEO >> 12;
    } else {
        page_table_kb_entry_t *usr_vid_table = (page_table_kb_entry_t *)tasks[cur_task]->usr_vid_table;
        usr_vid_table->addr = (uint32_t)terminal_video[tasks[cur_task]->terminal] >> 12;
    }

    switch_page_directory(cur_task);

    tasks[cur_task]->file_descs = file_desc_arrays[cur_task];

    sys_open((uint8_t *)"/dev/stdin");
    sys_open((uint8_t *)"/dev/stdout");

    // Parse arguments from command. command should still be on this stack
    // when getargs is called so just store a pointer to the correct offset.
    uint32_t i = 0;
    while(com_str[i] != ' ' && com_str[i] != '\0') i++;
    com_str[i] = '\0';
    i++;
    while(com_str[i] == ' ') i++;
    tasks[cur_task]->arg_str = com_str + i;

    // If the file cannot be found error
    if ((fd = sys_open(com_str)) == -1) {
        cur_task = tasks[cur_task]->parent;
        switch_page_directory(cur_task);
        return -1;
    }

    // Clear user memory (we don't want to leave data from previous processes
    // as that could be a huge vulnerability)
    memset((void*)TASK_ADDR, 0, MB4);

    int8_t *buf = (int8_t*)(TASK_ADDR + USR_CODE_OFFSET);

    fstat_t stats;
    sys_stat(fd, &stats, sizeof(fstat_t));

    sys_read(fd, (void*)buf, stats.size);
    sys_close(fd);

    // Magic executable bytes
    if (buf[0] != 0x7F || buf[1] != 0x45 || buf[2] != 0x4C || buf[3] != 0x46) {
        // File is not executable
        cur_task = tasks[cur_task]->parent;
        return -1;
    }

    tasks[cur_task]->status = TASK_RUNNING;
    tasks[tasks[cur_task]->parent]->status = TASK_SLEEPING;

    // A pointer to the first instruction is stored in bytes 24-27
    uint32_t start = ((uint32_t *)buf)[6];

    tss.esp0 = tasks[cur_task]->kernel_esp;

    tasks[cur_task]->user_esp = TASK_ADDR + MB4;

    // Setup an iret context on the stack with user CS and DS,
    // an EIP of start and an ESP of user_stack_addr
    // NOTE: in order to enable interrupts we OR the
    // sti flag (0x200) on EFLAGS so that iret will sti for us.
    asm volatile("                             \n\
    movw $" str(USER_DS) ", %%ax               \n\
    movw %%ax, %%ds                            \n\
    movw %%ax, %%es                            \n\
    movw %%ax, %%fs                            \n\
    movw %%ax, %%gs                            \n\
                                               \n\
    pushl $" str(USER_DS) "                    \n\
    pushl %1                                   \n\
    pushf                                      \n\
    popl %%eax                                 \n\
    orl $0x200, %%eax                          \n\
    pushl %%eax                                \n\
    pushl $" str(USER_CS) "                    \n\
    pushl %0                                   \n\
    iret                                       \n\
    "
                 :
                 : "b"(start), "c"(tasks[cur_task]->user_esp));
    return 0;
}

/* sys_read
 * Description: reads nbytes from the file pointed to by fd into buf
 * Input:  fd - the file descriptor to read from
 *         buf - the buffer to read into
 *         nbytes - the number of bytes to read
 * Output: -1 on error, number of bytes read otherwise
 * Side Effects: none
 */
int32_t sys_read(int32_t fd, void* buf, int32_t nbytes) {
    if (fd < 0 || fd > FILE_DESCS_LENGTH) {
        return -1;
    }
    switch (tasks[cur_task]->file_descs[fd].flags) {
    case FD_DIR:
    case FD_FILE:
    case FD_RTC:
    case FD_KBD:
    case FD_STDIN:
        return (*tasks[cur_task]->file_descs[fd].ops->read)(fd, buf, nbytes);
    default:
        return -1;
    }
}

/* sys_write
 * Description: writes nbytes of buf into the file pointed to by fd (unimplemented on filesystem)
 * Input:  fd - the file descriptor to write to
 *         buf - the buffer to write from
 *         nbytes - the number of bytes to write
 * Output: -1 on error, number of bytes written otherwise
 * Side Effects: writes to fd
 */
int32_t sys_write(int32_t fd, const void* buf, int32_t nbytes) {
    if (fd < 0 || fd > FILE_DESCS_LENGTH) {
        return -1;
    }
    switch (tasks[cur_task]->file_descs[fd].flags) {
    case FD_RTC:
    case FD_STDOUT:
        return (*tasks[cur_task]->file_descs[fd].ops->write)(fd, buf, nbytes);
    default:
        return -1;
    }
}

/* sys_open
 * Description: opens the file filename for cur_task and returns an fd for it
 * Input:  filename - the file to open
 * Output: -1 on error, fd otherwise
 * Side Effects: writes to tasks[cur_task]->file_descs
 */
int32_t sys_open(const uint8_t* filename) {
    if (!strncmp((int8_t*)filename, "/dev/stdin", strlen("/dev/stdin"))) {
        tasks[cur_task]->file_descs[0].ops = &stdin_ops;
        tasks[cur_task]->file_descs[0].inode = NULL;
        tasks[cur_task]->file_descs[0].flags = FD_STDIN;
        tasks[cur_task]->file_descs[0].ops->open((int8_t*)filename);
        return 0;
    }
    if (!strncmp((int8_t*)filename, "/dev/stdout", strlen("/dev/stdout"))) {
        tasks[cur_task]->file_descs[1].ops  = &stdout_ops;
        tasks[cur_task]->file_descs[1].inode = NULL;
        tasks[cur_task]->file_descs[1].flags = FD_STDOUT;
        tasks[cur_task]->file_descs[1].ops->open((int8_t*)filename);
        return 1;
    }
    int i;
    for (i = 2; i < FILE_DESCS_LENGTH; i++) {
        if (tasks[cur_task]->file_descs[i].flags == FD_CLEAR) {
            break;
        }
    }
    if (i < FILE_DESCS_LENGTH) {
        if (!strncmp((int8_t*)filename, "rtc", strlen("rtc"))) {
            tasks[cur_task]->file_descs[i].ops = &rtc_ops;
            tasks[cur_task]->file_descs[i].inode = NULL;
            tasks[cur_task]->file_descs[i].flags = FD_RTC;

            tasks[cur_task]->file_descs[i].ops->open((int8_t*)filename);
            return i;
        }

        if (!strncmp((int8_t*)filename, "/dev/kbd", strlen("/dev/kbd"))) {
            tasks[cur_task]->file_descs[i].ops = &kbd_ops;
            tasks[cur_task]->file_descs[i].inode = NULL;
            tasks[cur_task]->file_descs[i].flags = FD_KBD;

            tasks[cur_task]->file_descs[i].ops->open((int8_t*)filename);
            return i;
        }

        tasks[cur_task]->file_descs[i].inode = (*filesys_ops.open)((int8_t*)filename);
        if (tasks[cur_task]->file_descs[i].inode == -1) {
            return -1;
        }
        dentry_t d;
        if (read_dentry_by_name((int8_t*)filename, &d) == 0) {

            tasks[cur_task]->file_descs[i].ops = &filesys_ops;
            switch (d.type) {
            case FD_DIR:
                if (-1 == (tasks[cur_task]->file_descs[i].file_pos = get_index((int8_t*)filename))) {
                    return -1;
                }
                tasks[cur_task]->file_descs[i].flags = FD_DIR;
                break;
            case FD_FILE:
                tasks[cur_task]->file_descs[i].file_pos = 0;
                tasks[cur_task]->file_descs[i].flags = FD_FILE;
                break;
            default:
                return -1;
            }
        } else {
            return -1;
        }

        return i;
    } else {
        return -1;
    }
}

int32_t sys_close(int32_t fd) {
    if (fd >= FILE_DESCS_LENGTH || fd < 2) {
        return -1;
    }

    if (tasks[cur_task]->file_descs[fd].flags == FD_CLEAR) {
        return -1;
    }

    tasks[cur_task]->file_descs[fd].flags = FD_CLEAR;
    return (*tasks[cur_task]->file_descs[fd].ops->close)(fd);
}

int32_t sys_getargs(uint8_t* buf, int32_t nbytes) {
    uint8_t* arg = tasks[cur_task]->arg_str;
    int32_t arg_len = strlen((int8_t*)arg) + 1;
    if (nbytes < arg_len) {
        return -1;
    }
    memcpy(buf, arg, arg_len);
    return 0;
}

int32_t sys_vidmap(uint8_t** screen_start) {
    if ((uint32_t)screen_start < TASK_ADDR || (uint32_t)screen_start >= (TASK_ADDR + MB4)) {
        return -1;
    }

    *screen_start = (uint8_t *)(TASK_ADDR + MB4);

    return 0;
}

int32_t sys_set_handler(int32_t signum, void* handler_address) {
    if ((uint32_t)handler_address < TASK_ADDR || (uint32_t)handler_address >= (TASK_ADDR + MB4)) {
        return -1;
    }
    signal_handlers[cur_task][signum] = handler_address;
    return 0;
}

int32_t sys_sigreturn(void) {
    tasks[cur_task]->signal_mask = false;

    if (tasks[cur_task]->sig_hw_context->iret_context.cs == KERNEL_CS) {
        hw_context_t *hw_context = (hw_context_t *)(tasks[cur_task]->ebp + 12);
        tss.esp0 = tasks[cur_task]->kernel_esp;
        memcpy(hw_context, (void*)tasks[cur_task]->sig_hw_context, sizeof(hw_context_t) - 8);
        uint32_t ebp = tasks[cur_task]->ebp;
        asm volatile("movl %0, %%ebp; leave; ret;"
                     : : "b"(ebp));
    } else {
        uint32_t ebp;
        asm volatile("movl %%ebp, %0;" : "=r"(ebp) :);
        hw_context_t *hw_context = (hw_context_t *)(ebp + 20);
        memcpy(hw_context, (void*)tasks[cur_task]->sig_hw_context, sizeof(hw_context_t));
        return hw_context->eax;
    }

    // Stifle errors
    return 0;
}

int32_t sys_vidmap_all(uint8_t** screen_start) {
    if ((uint32_t)screen_start < TASK_ADDR || (uint32_t)screen_start >= (TASK_ADDR + MB4)) {
        return -1;
    }

    uint32_t vid_mem = 0xA0000;
    uint8_t i = 0;
    for (i = 0; i < 32; i++) {
        page_table_kb_entry_t* vid_entry = (page_table_kb_entry_t *)&tasks[cur_task]->usr_vid_table[i];
        vid_entry->addr = (vid_mem >> 12);
        vid_entry->avail = 0;
        vid_entry->global = 0;
        vid_entry->pgTblAttIdx = 0;
        vid_entry->dirty = 0;
        vid_entry->accessed = 0;
        vid_entry->cacheDisabled = 0;
        vid_entry->writeThrough = 1;  //1 for fun
        vid_entry->userSupervisor = 1;
        vid_entry->readWrite = 1;     //Write enabled
        vid_entry->present = 1;

        vid_mem += 0x1000;
    }

    switch_page_directory(cur_task);

    *screen_start = (uint8_t *)(TASK_ADDR + MB4);

    return 0;
}

int32_t sys_ioperm(uint32_t from, uint32_t num, int32_t turn_on) {
    // TODO: Use the TSS to avoid giving permisions for all ports
    uint32_t *ebp;
    asm volatile("movl %%ebp, %0;" : "=r"(ebp) : );
    ebp[19] |= 0x3000;

    return 0;
}

int32_t sys_thread_create(uint32_t *tid, void (*thread_start)()) {
    cli();
    uint32_t ebp;
    asm volatile("movl %%ebp, %0;" : "=r"(ebp) : );
    tasks[cur_task]->ebp = ebp;
    tasks[cur_task]->kernel_esp = ebp-4;

    uint8_t task_num;
    for (task_num = 1; task_num < NUM_TASKS; task_num++) {
        if (tasks[task_num]->status == TASK_EMPTY) {
            break;
        }
    }

    if (task_num >= NUM_TASKS) {
        return -1;
    }

    tasks[task_num]->status = TASK_RUNNING;
    tasks[task_num]->file_descs = tasks[cur_task]->file_descs;
    memcpy(tasks[task_num]->page_directory, tasks[cur_task]->page_directory, DIR_SIZE * 4);
    // Add a page directory entry mapping 136MB virtual to the physical space this task would have taken up.
    // This will be used for the user level stack.
    setup_task_mem(tasks[task_num]->page_directory + 34, task_num);
    memcpy(tasks[task_num]->usr_vid_table, tasks[cur_task]->usr_vid_table, DIR_SIZE * 4);
    memcpy(tasks[task_num]->kernel_vid_table, tasks[cur_task]->kernel_vid_table, DIR_SIZE * 4);
    tasks[task_num]->arg_str = NULL;
    tasks[task_num]->terminal = tasks[cur_task]->terminal;
    tasks[task_num]->rtc_counter = 0;
    tasks[task_num]->rtc_base = tasks[cur_task]->rtc_base;
    tasks[task_num]->parent = cur_task;
    *tid = task_num;
    SET_THREAD(cur_task, task_num);
    tasks[task_num]->thread_status = 1;
    tasks[task_num]->kernel_esp = (uint32_t)&task_stacks[task_num].stack_start;

    cur_task = task_num;
    switch_page_directory(cur_task);
    tss.esp0 = tasks[cur_task]->kernel_esp;

    uint8_t *uesp = (uint8_t *)(MB4 * 35 - 4);

    // Put the following assembly on the user stack:
    // pushl $0x1, %eax
    // int $0x80
    *uesp = 0x00; uesp--;
    *uesp = 0x80; uesp--;
    *uesp = 0xCD; uesp--;
    *uesp = 0x00; uesp--;

    *uesp = 0x00; uesp--;
    *uesp = 0x00; uesp--;
    *uesp = 0x01; uesp--;
    *uesp = 0xB8;

    // Put a pointer to the assembly on the stack so that
    // when the thread returns it starts executing the assembly
    uint32_t return_addr = (uint32_t)uesp;
    uesp -= 5;
    *(uint32_t *)uesp = return_addr;

    tasks[cur_task]->user_esp = (uint32_t)uesp;

    asm volatile("                             \n\
    movw $" str(USER_DS) ", %%ax               \n\
    movw %%ax, %%ds                            \n\
    movw %%ax, %%es                            \n\
    movw %%ax, %%fs                            \n\
    movw %%ax, %%gs                            \n\
                                               \n\
    pushl $" str(USER_DS) "                    \n\
    pushl %1                                   \n\
    pushf                                      \n\
    popl %%eax                                 \n\
    orl $0x200, %%eax                          \n\
    pushl %%eax                                \n\
    pushl $" str(USER_CS) "                    \n\
    pushl %0                                   \n\
    iret                                       \n\
    "
                 :
                 : "b"(thread_start), "c"(uesp));
    return 0;
}

int32_t sys_thread_join(uint32_t tid) {
    if (tasks[tid]->status == TASK_ZOMBIE) {
        tasks[tid]->status = TASK_EMPTY;
        CLEAR_THREAD(cur_task, tid);
    } else {
        tasks[cur_task]->thread_waiting = tid;
        tasks[cur_task]->status = TASK_WAITING_FOR_THREAD;

        reschedule();
    }
    tasks[tid]->page_directory[34] = 2;
    return 0;
}

int32_t sys_stat(int32_t fd, void* buf, int32_t nbytes) {
    switch (tasks[cur_task]->file_descs[fd].flags) {
    case FD_FILE:
    case FD_DIR:
        return (*tasks[cur_task]->file_descs[fd].ops->stat)(fd, buf, nbytes);
    default:
        return -1;
    }
}

int32_t sys_time(uint32_t* buf){
    *buf = get_time();
    return 0;
}
