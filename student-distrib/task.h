#ifndef TASK_H_
#define TASK_H_

#include "types.h"

void create_init();

#define FILE_DESCS_LENGTH 8
#define NUM_TASKS 10

typedef struct file_ops {
    int32_t (*open)(const int8_t*);
    int32_t (*close)(int32_t);
    int32_t (*read)(int32_t, void*, int32_t);
    int32_t (*write)(int32_t, const void*, int32_t);
    int32_t (*stat)(int32_t, void*, int32_t);
} file_ops_t;

file_ops_t filesys_ops;


#define FD_CLEAR 0
#define FD_DIR 1
#define FD_FILE 2
#define FD_STDIN 3
#define FD_STDOUT 4
#define FD_RTC 5
#define FD_KBD 6

typedef struct file_desc {
    file_ops_t *ops;
    int32_t inode;
    int32_t file_pos;
    int32_t flags;
} file_desc_t;

#define TASK_EMPTY 0
#define TASK_RUNNING 1
#define TASK_SLEEPING 2

#define INIT 0



typedef struct {
    int32_t status;
    file_desc_t file_descs[FILE_DESCS_LENGTH];
    uint32_t *page_directory;
    uint32_t *kernel_vid_table;
    uint32_t *usr_vid_table;
    uint32_t ebp;
    uint8_t parent;
    uint32_t kernel_esp;
    uint8_t* arg_str;
    uint32_t terminal;
    bool rtc_flag;
} pcb_t;

#define KERNEL_STACK_SIZE 0x8000

// This allocates enough space for a kernel stack and puts a pcb_t at the end of it
typedef struct {
    pcb_t pcb;
    uint8_t stack[KERNEL_STACK_SIZE - sizeof(pcb_t) - 1];
    uint8_t stack_start;
} kernel_stack_t;

// Let the linker know how much space all the kernel stacks will take up
kernel_stack_t task_stacks[NUM_TASKS] __attribute__((aligned (KERNEL_STACK_SIZE)));

pcb_t *tasks[NUM_TASKS];

extern uint8_t cur_task;

#endif
