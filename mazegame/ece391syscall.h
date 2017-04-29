#if !defined(ECE391SYSCALL_H)
#define ECE391SYSCALL_H

#include <stdint.h>

/* All calls return >= 0 on success or -1 on failure. */

/*
 * Note that the system call for halt will have to make sure that only
 * the low byte of EBX (the status argument) is returned to the calling
 * task.  Negative returns from execute indicate that the desired program
 * could not be found.
 */
extern int32_t halt (uint8_t status);
extern int32_t execute (const uint8_t* command);
extern int32_t read (int32_t fd, void* buf, int32_t nbytes);
extern int32_t write (int32_t fd, const void* buf, int32_t nbytes);
extern int32_t open (const uint8_t* filename);
extern int32_t close (int32_t fd);
extern int32_t getargs (uint8_t* buf, int32_t nbytes);
extern int32_t vidmap (uint8_t** screen_start);
extern int32_t set_handler (int32_t signum, void* handler);
extern int32_t sigreturn (void);
extern int32_t vidmap_all (uint8_t **screen_start);
extern int32_t ioperm(uint32_t from, uint32_t num, int32_t turn_on);
extern int32_t thread_create(uint32_t *tid, void (*thread_start)());
extern int32_t thread_join(uint32_t tid);
extern int32_t time(uint32_t* buf);

enum signums {
    DIV_ZERO = 0,
    SEGFAULT,
    INTERRUPT,
    ALARM,
    USER1,
    NUM_SIGNALS
};

#endif /* ECE391SYSCALL_H */
