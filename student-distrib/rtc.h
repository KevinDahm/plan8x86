#ifndef _RTC_H
#define _RTC_H

#include "types.h"
#include "filesystem.h"
#include "task.h"

#define RTC_PORT 0x70
#define CHOOSE_RTC_A 0x8A
#define CHOOSE_RTC_B 0x8B
#define CHOOSE_RTC_C 0x0C
#define BASE_RTC_FREQ 2
#define MAX_RTC_FREQ 1024
#define RTC_CMD_A 0x20
#define RTC_CMD_B 0x40

/*Call to initialize RTC*/
extern void rtc_init();
//system call to open rtc
extern int32_t rtc_open(const int8_t* filename);
//system call to close rtc
extern int32_t rtc_close(int32_t fd);
//system call to write to rtc
extern int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);
//system call to read from rtc
extern int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);
//system call to get stats from rtc
extern int32_t rtc_stat(int32_t fd, void* buf, int32_t nbytes);
extern uint32_t get_time();

//struct to store system call function pointers
file_ops_t rtc_ops;
#endif
