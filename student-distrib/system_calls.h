#ifndef SYSTEM_CALLS_H_
#define SYSTEM_CALLS_H_

#include "types.h"

extern bool backup_init_ebp;
//Stops the process that called this and returns control to the proccess that ran sys_execute
extern int32_t sys_halt(uint32_t status);

// Starts a new process specified by command
extern int32_t sys_execute(const uint8_t* command);

//reads nbytes from the file pointed to by fd into buf
extern int32_t sys_read(int32_t fd, void* buf, int32_t nbytes);

//writes nbytes of buf into the file pointed to by fd (unimplemented on filesystem)
extern int32_t sys_write(int32_t fd, const void* buf, int32_t nbytes);

//opens the file filename for cur_task and returns an fd for it
extern int32_t sys_open(const uint8_t* filename);

//closes the file filename for cur_task
extern int32_t sys_close(int32_t fd);

// copies the string arguments for a task to buf
extern int32_t sys_getargs(uint8_t* buf, int32_t nbytes);

//makes *screen_start a pointer to video memory
extern int32_t sys_vidmap(uint8_t** screen_start);

//makes handler_address the signal handler for signal signum for the task
extern int32_t sys_set_handler(int32_t signum, void* handler_address);

//returns from a user signal handler
extern int32_t sys_sigreturn(void);

//makes *screen_start a pointer to video memory for ALL of VGA
extern int32_t sys_vidmap_all(uint8_t** screen_start);

//gives the user access to all io ports
extern int32_t sys_ioperm(uint32_t from, uint32_t num, int32_t turn_on);

//starts a new thread at function thread_start and stores its id in td
extern int32_t sys_thread_create(uint32_t *tid, void (*thread_start)());

// waits for a child thread to exit
extern int32_t sys_thread_join(uint32_t tid);

// writes file stats to buf
extern int32_t sys_stat(int32_t fd, void* buf, int32_t nbytes);

//returns time since system boot
extern uint32_t sys_time();


#endif
