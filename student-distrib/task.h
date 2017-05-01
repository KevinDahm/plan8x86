#ifndef TASK_H_
#define TASK_H_

#include "types.h"
#include "schedule.h"

//Initializes paging for the kernel and each user task and sets up default values for each task
void create_init();

// Sets up a 4KB page mapping to video memory
void setup_vid(uint32_t *dir, uint32_t *table, uint32_t priv);

//Fills in a 4MB page directory entry at dir (which provides the virtual address) and maps it to physical address 4MB
void setup_kernel_mem(uint32_t *dir);
//Fills in a 4MB page directory entry at dir (which provides the virtual address) and maps it to physical address
void setup_task_mem(uint32_t *dir, uint32_t task);

#define FILE_DESCS_LENGTH 8
#define NUM_TASKS 10

//stub functions - default for file_ops
int32_t default_open(const int8_t *buf);
int32_t default_close(int32_t fd);
int32_t default_read(int32_t fd, void *buf, int32_t nbytes);
int32_t default_write(int32_t fd, const void *buf, int32_t nbytes);
int32_t default_stat(int32_t fd, void *buf, int32_t nbytes);

typedef struct file_ops {
    int32_t (*open)(const int8_t*);
    int32_t (*close)(int32_t);
    int32_t (*read)(int32_t, void*, int32_t);
    int32_t (*write)(int32_t, const void*, int32_t);
    int32_t (*stat)(int32_t, void*, int32_t);
} file_ops_t;

file_ops_t default_ops;

file_ops_t filesys_ops;


#define FD_CLEAR 0
#define FD_RTC 5
#define FD_DIR 1
#define FD_FILE 2
#define FD_STDIN 3
#define FD_STDOUT 4
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
#define TASK_ZOMBIE 3
#define TASK_WAITING_FOR_THREAD 4

enum signals {
    DIV_ZERO = 0,
    SEGFAULT,
    INTERRUPT,
    ALARM,
    USER1,
    NUM_SIGNALS
};

#define SET_SIGNAL(task, signal) do {tasks[task]->pending_signals |= (1 << signal);} while(0)
#define CLEAR_SIGNAL(task, signal) do {tasks[task]->pending_signals &= ~(1 << signal);} while(0)
#define SIGNAL_SET(task, signal) ((tasks[task]->pending_signals & (1 << signal)) != 0)

// signal_handlers is an array of NUM_TASKS of arrays of NUM_SIGNALS
// of function pointers to functions taking int32_t and returning void
void (*signal_handlers[NUM_TASKS][NUM_SIGNALS])(int32_t);

#define INIT 0

file_desc_t file_desc_arrays[NUM_TASKS][FILE_DESCS_LENGTH];

#define CLEAR_THREAD(task, tid) do {tasks[task]->thread_status &= ~(1 << tid);} while(0)
#define SET_THREAD(task, tid) do {tasks[task]->thread_status |= (1 << tid);} while(0)

typedef struct {
    // Can be any of TASK_EMPTY, TASK_RUNNING, TASK_SLEEPING,
    // TASK_ZOMBIE, or TASK_WAITING_FOR_THREAD. Used to manage
    // scheduling, threading, and creating new tasks
    int32_t status;
    // An array of files owned by the process
    file_desc_t *file_descs;
    // Pointers to the page directories and tables associated with this process
    uint32_t *page_directory;
    uint32_t *kernel_vid_table;
    uint32_t *usr_vid_table;
    // ebp is used for returning to interrupted processes
    uint32_t ebp;
    // A pointer to the kernel stack that this process should be using
    uint32_t kernel_esp;
    // A pointer to the user stack that this process is using
    // This is used for signals and threading to know where to
    // push data to and maintained by backup_uesp
    uint32_t user_esp;
    // A pointer to the arguments given to execute for retrieval by sys_getargs
    uint8_t* arg_str;
    // The terminal this process belongs to
    uint32_t terminal;
    // Any pending signals waiting to be delivered will have a bit set here
    // The format is (1 << signum). Please use SET_SIGNAL, CLEAR_SIGNAL, and SIGNAL_SET
    uint32_t pending_signals;
    // A pointer to the hardware context on the user stack put there before a signal
    // handler is called. Used by sys_sigreturn.
    hw_context_t *sig_hw_context;
    // If thread_status is 0 this process is a regular process with no threads
    // If thread_status is 1 this process is a thread
    // If thread_status is >1 this process owns threads where each bit set in thread_status
    // corresponds to the index in tasks of the owned thread.
    uint32_t thread_status;
    // Number of rtc interupts needed to return from rtc read
    int32_t rtc_base;
    // counts down on each rtc interrupt until it reaches 0 at which point
    // rtc read will return and this will get reset to rtc_base
    int32_t rtc_counter;
    // If a parent process is waiting for a thread to finish (after sys_thread_join is called)
    // The tid of the thread will be here. Used by sys_halt
    uint8_t thread_waiting;
    // An index into tasks of the parent process
    uint8_t parent;
    // Whether or not signals are masked for this process.
    bool signal_mask;
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
