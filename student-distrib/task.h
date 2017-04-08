#ifndef TASK_H_
#define TASK_H_

#include "types.h"
#include "page.h"

void create_init();

#define FILE_DESCS_LENGTH 8
#define NUM_TASKS 3

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

/* typedef struct { */
/*     uint32_t xss; */
/*     uint32_t esp; */
/*     uint32_t eflags; */
/*     uint32_t ebx; */
/*     uint32_t ecx; */
/*     uint32_t edx; */
/*     uint32_t esi; */
/*     uint32_t edi; */
/*     uint32_t ebp; */
/*     uint32_t eax; */
/*     uint32_t xds; */
/*     uint32_t xes; */
/*     uint32_t xfs; */
/*     uint32_t eip; */
/*     uint32_t xcs; */
/* } regs_t; */

typedef struct {
    uint32_t ebp;
    /* uint32_t esp; */
} regs_t;

typedef struct {
    int32_t status;
    file_desc_t file_descs[FILE_DESCS_LENGTH];
    uint32_t page_directory_tables[DIR_SIZE] __attribute__((aligned (0x1000)));
    uint32_t page_tables[DIR_SIZE] __attribute__((aligned (0x1000)));
    regs_t regs;
    uint8_t parent;
} pcb_t;

pcb_t tasks[NUM_TASKS];

uint8_t cur_task;

#endif
