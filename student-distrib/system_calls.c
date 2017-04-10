#include "system_calls.h"
#include "filesystem.h"
#include "lib.h"
#include "rtc.h"
#include "kbd.h"
#include "terminal.h"
#include "page.h"
#include "x86_desc.h"
#include "task.h"

#define st(a) #a
#define str(a) st(a)

uint32_t halt_status;

int32_t sys_halt(uint32_t status) {
    int i;
    for (i = 0; i < FILE_DESCS_LENGTH; i++) {
        if (tasks[cur_task]->file_descs[i].flags != FD_CLEAR) {
            sys_close(i);
        }
    }

    tasks[cur_task]->status = TASK_EMPTY;

    cur_task = tasks[cur_task]->parent;

    switch_page_directory(cur_task);

    if (cur_task == 0) {
        sys_execute((uint8_t*)"shell");
    }

    uint32_t ebp = tasks[cur_task]->regs.ebp;

    // Save the return value before moving the stack.
    // halt_status has to be static memory, it cannot be on the stack.
    halt_status = status;

    asm volatile(" \n\
    movl %0, %%ebp \n"
                 :
                 : "r"(ebp));

    tss.esp0 = tasks[cur_task]->kernel_esp;

    return halt_status;
}

int32_t sys_execute(const uint8_t* command) {
    uint8_t com_str[strlen((int8_t*)command)];
    strcpy((int8_t*)com_str, (int8_t*)command);

    uint32_t ebp;

    asm volatile(" \n\
    movl %%ebp, %0 \n"
                 : "=r"(ebp)
                 :);
    tasks[cur_task]->regs.ebp = ebp;
    /* tasks[cur_task].regs.esp = ebp + 4; */
    /* r -= 20; */
    /* memcpy(&tasks[cur_task].regs, r, sizeof(regs_t)); */


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

    tasks[task_num]->parent = cur_task;
    cur_task = task_num;
    tasks[cur_task]->status = TASK_RUNNING;

    sys_open((uint8_t *)"/dev/stdin");
    sys_open((uint8_t *)"/dev/stdout");

    // TODO: Parse command
    uint32_t i = 0;
    while(com_str[i] != ' ' && com_str[i] != '\0') i++;
    com_str[i] = '\0';
    i++;
    while(com_str[i] == ' ') i++;
    tasks[cur_task]->arg_str = com_str + i;

    if ((fd = sys_open(com_str)) == -1) {
        return -1;
    }

    fstat_t stats;
    sys_stat(fd, &stats, sizeof(fstat_t));

    switch_page_directory(cur_task);

    int8_t *buf = (int8_t*)(TASK_ADDR + USR_CODE_OFFSET);

    sys_read(fd, (void*)buf, stats.size);
    sys_close(fd);

    if (buf[0] != 0x7F || buf[1] != 0x45 || buf[2] != 0x4C || buf[3] != 0x46) {
        // File is not executable
        return -1;
    }

    uint32_t start = ((uint32_t *)buf)[6];

    tss.esp0 = tasks[cur_task]->kernel_esp;

    uint32_t user_stack_addr = TASK_ADDR + MB4;

    asm volatile("                             \n\
    cli                                        \n\
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
                 : "b"(start), "c"(user_stack_addr));
    return 0;
}

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

int32_t sys_open(const uint8_t* filename) {
    if (!strncmp((int8_t*)filename, "/dev/stdin", strlen("/dev/stdin"))) {
        tasks[cur_task]->file_descs[0].ops = &stdin_ops;
        tasks[cur_task]->file_descs[0].inode = NULL;
        tasks[cur_task]->file_descs[0].flags = FD_STDIN;
        return 0;
    }
    if (!strncmp((int8_t*)filename, "/dev/stdout", strlen("/dev/stdout"))) {
        tasks[cur_task]->file_descs[1].ops  = &stdout_ops;
        tasks[cur_task]->file_descs[1].inode = NULL;
        tasks[cur_task]->file_descs[1].flags = FD_STDOUT;
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
            return i;
        }

        if (!strncmp((int8_t*)filename, "/dev/kbd", strlen("/dev/kbd"))) {
            tasks[cur_task]->file_descs[i].ops = &kbd_ops;
            tasks[cur_task]->file_descs[i].inode = NULL;
            tasks[cur_task]->file_descs[i].flags = FD_KBD;
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
            case 1:
                tasks[cur_task]->file_descs[i].file_pos = get_index((int8_t*)filename);
                tasks[cur_task]->file_descs[i].flags = FD_DIR;
                break;
            case 2:
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

    tasks[cur_task]->file_descs[fd].flags = 0;
    return (*tasks[cur_task]->file_descs[fd].ops->close)(fd);
}

int32_t sys_getargs(uint8_t* buf, int32_t nbytes) {
    uint8_t* arg = tasks[cur_task]->arg_str;
    uint32_t arg_len = strlen((int8_t*)arg);
    uint32_t out_len = nbytes < arg_len ? nbytes : arg_len;
    memcpy(buf, arg, out_len);
    return 0;
}

int32_t sys_vidmap(uint8_t** screen_start) {
    if ((uint32_t)screen_start < TASK_ADDR || (uint32_t)screen_start >= (TASK_ADDR + MB4)) {
        return -1;
    }

    /* page_table_kb_entry_t* video_entry = */
    /*     (page_table_kb_entry_t*)(tasks[cur_task]->page_table + ); */
    /* video_entry->addr = VIDEO >> 12;    //Lose lower 12 bits (keep 20 high bits) */
    /* video_entry->avail = 0; */
    /* video_entry->global = 0; */
    /* video_entry->pgTblAttIdx = 0; */
    /* video_entry->dirty = 0; */
    /* video_entry->accessed = 0; */
    /* video_entry->cacheDisabled = 0; */
    /* video_entry->writeThrough = 1;  //1 for fun */
    /* video_entry->userSupervisor = 0; */
    /* video_entry->readWrite = 1;     //Write enabled */
    /* video_entry->present = 1; */
    return 0;
}

int32_t sys_set_handler(int32_t signum, void* handler_address) {
    return -1;
}

int32_t sys_sigreturn(void) {
    return -1;
}

// TODO: processes
void system_calls_init() {
    cur_task = 0;
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
