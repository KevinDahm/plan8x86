#ifndef SYSTEM_CALLS_H_
#define SYSTEM_CALLS_H_

#include "types.h"

extern bool backup_init_ebp;

extern int32_t sys_halt(uint32_t status);
extern int32_t sys_execute(const uint8_t* command);
extern int32_t sys_read(int32_t fd, void* buf, int32_t nbytes);
extern int32_t sys_write(int32_t fd, const void* buf, int32_t nbytes);
extern int32_t sys_open(const uint8_t* filename);
extern int32_t sys_close(int32_t fd);
extern int32_t sys_getargs(uint8_t* buf, int32_t nbytes);
extern int32_t sys_vidmap(uint8_t** screen_start);
extern int32_t sys_set_handler(int32_t signum, void* handler_address);
extern int32_t sys_sigreturn(void);
extern int32_t sys_vidmap_all(uint8_t** screen_start);
extern int32_t sys_ioperm(uint32_t from, uint32_t num, int32_t turn_on);
extern int32_t sys_thread_create(uint32_t *tid, void (*thread_start)());
extern int32_t sys_thread_join(uint32_t tid);

extern int32_t sys_stat(int32_t fd, void* buf, int32_t nbytes);
extern int32_t sys_time(uint32_t* buf);


#endif
